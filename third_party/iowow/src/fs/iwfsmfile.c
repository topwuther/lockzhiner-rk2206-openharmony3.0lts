/**************************************************************************************************
 * IOWOW library
 *
 * MIT License
 *
 * Copyright (c) 2012-2020 Softmotions Ltd <info@softmotions.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *************************************************************************************************/

#include "iwcfg.h"
#include "platform/iwp.h"
#include "log/iwlog.h"
#include "iwfsmfile.h"
#include "utils/iwutils.h"
#include "utils/kbtree.h"
#include "utils/iwbits.h"

#include <pthread.h>

typedef struct IWFS_FSM_IMPL FSM;

void iwfs_fsmdbg_dump_fsm_tree(IWFS_FSM *f, const char *hdr);

/**
 * Free-space blocks-tree key.
 */
typedef struct {
  uint32_t off;
  uint32_t len;
} FSMBK;

/** Additional options for `_fsm_set_bit_status_lw` routine */
typedef uint8_t fsm_bmopts_t;
/** No options. */
#define FSM_BM_NONE     ((fsm_bmopts_t) 0x00U)
/** Do not modify bitmap. */
#define FSM_BM_DRY_RUN  ((fsm_bmopts_t) 0x01U)
/** Perform strict checking of bitmap consistency */
#define FSM_BM_STRICT   ((fsm_bmopts_t) 0x02U)

/* Maximum size of block: 1Mb */
#define FSM_MAX_BLOCK_POW 20

/* Maximum number of records used in allocation statistics */
#define FSM_MAX_STATS_COUNT 0x0000ffff

#define FSM_ENSURE_OPEN(impl_)                                                                          \
  if (!(impl_) || !(impl_)->f) return IW_ERROR_INVALID_STATE;

#define FSM_ENSURE_OPEN2(f_)                                                                             \
  if (!(f_) || !(f_)->impl) return IW_ERROR_INVALID_STATE;

#define FSMBK_OFFSET(b_) ((b_)->off)

#define FSMBK_LENGTH(b_) ((b_)->len)

////////////////////////////////////////////////////////////////////////////////////////////////////

IW_INLINE int _fsm_cmp_ptr(const FSMBK *a, const FSMBK *b);
#define _fsm_cmp(a_, b_) (_fsm_cmp_ptr(&(a_), &(b_)))

// -V:KBTREE_INIT:522, 641
KBTREE_INIT(fsm, FSMBK, _fsm_cmp)

struct IWFS_FSM_IMPL {
  IWFS_EXT pool;             /**< Underlying rwl file. */
  uint64_t bmlen;            /**< Free-space bitmap block length in bytes. */
  uint64_t bmoff;            /**< Free-space bitmap block offset in bytes. */
  uint64_t lfbkoff;          /**< Offset in blocks of free block chunk with the largest offset. */
  uint64_t lfbklen;          /**< Length in blocks of free block chunk with the largest offset. */
  uint64_t crzsum;           /**< Cumulative sum all allocated blocks */
  uint64_t crzvar;           /**< Record sizes standard variance (deviation^2 * N) */
  uint32_t hdrlen;           /**< Length of custom file header */
  uint32_t crznum;           /**< Number of all allocated continuous areas acquired by `allocate` */
  IWFS_FSM *f;               /**< Self reference. */
  IWDLSNR *dlsnr;            /**< Data events listener */
  kbtree_t(fsm) *fsm;        /**< Free-space tree */
  pthread_rwlock_t *ctlrwlk; /**< Methods RW lock */
  size_t aunit;              /**< System allocation unit size.
                                  - Page size on *NIX
                                  - Minimal allocation unit for WIN32 */
  iwfs_fsm_openflags oflags; /**< Operation mode flags. */
  iwfs_omode omode;          /**< Open mode. */
  uint8_t bpow;              /**< Block size power for 2 */
  bool mmap_all;             /**< Mmap all file data */
  iwfs_ext_mmap_opts_t mmap_opts; /**< Defaul mmap options used in `add_mmap` */
};

static iwrc _fsm_ensure_size_lw(FSM *impl, off_t size);

////////////////////////////////////////////////////////////////////////////////////////////////////

IW_INLINE int _fsm_cmp_ptr(const FSMBK *a, const FSMBK *b) {
  int ret = ((FSMBK_LENGTH(b) < FSMBK_LENGTH(a)) - (FSMBK_LENGTH(a) < FSMBK_LENGTH(b)));
  if (ret) {
    return ret;
  } else {
    return ((FSMBK_OFFSET(b) < FSMBK_OFFSET(a)) - (FSMBK_OFFSET(a) < FSMBK_OFFSET(b)));
  }
}

IW_INLINE iwrc _fsm_ctrl_wlock(FSM *impl) {
  int rci = impl->ctlrwlk ? pthread_rwlock_wrlock(impl->ctlrwlk) : 0;
  return (rci ? iwrc_set_errno(IW_ERROR_THREADING_ERRNO, rci) : 0);
}

IW_INLINE iwrc _fsm_ctrl_rlock(FSM *impl) {
  int rci = impl->ctlrwlk ? pthread_rwlock_rdlock(impl->ctlrwlk) : 0;
  return (rci ? iwrc_set_errno(IW_ERROR_THREADING_ERRNO, rci) : 0);
}

IW_INLINE iwrc _fsm_ctrl_unlock(FSM *impl) {
  int rci = impl->ctlrwlk ? pthread_rwlock_unlock(impl->ctlrwlk) : 0;
  return (rci ? iwrc_set_errno(IW_ERROR_THREADING_ERRNO, rci) : 0);
}

IW_INLINE iwrc _fsm_bmptr(FSM *impl, uint64_t **bmptr) {
  size_t sp;
  uint8_t *mm;
  *bmptr = 0;
  // get mmap pointer without locked
  iwrc rc = impl->pool.probe_mmap(&impl->pool, impl->mmap_all ? 0 : impl->bmoff, &mm, &sp);
  RCRET(rc);
  if (impl->mmap_all) {
    if (sp < impl->bmoff + impl->bmlen) {
      return IWFS_ERROR_NOT_MMAPED;
    }
    *bmptr = (uint64_t *)(mm + impl->bmoff);
  } else {
    if (sp < impl->bmlen) {
      return IWFS_ERROR_NOT_MMAPED;
    }
    *bmptr = (uint64_t *) mm;
  }
  return 0;
}

/**
 * @brief Init the given @a bk key
 *        with given @a offset
 *        and @a length values.
 */
IW_INLINE WUR iwrc _fsm_init_fbk(FSMBK *bk, uint64_t offset_blk, uint64_t len_blk) {
  if (offset_blk > ((uint32_t) - 1) ||
      len_blk > ((uint32_t) - 1)) {
    return IW_ERROR_OVERFLOW;
  }
  bk->off = (uint32_t) offset_blk;
  bk->len = (uint32_t) len_blk;
  return 0;
}

/**
 * @brief Remove free space block from the fsm tree.
 * @param impl `FSM`
 * @param offset_blk Offset block number
 * @param length_blk Number of blocks
 */
IW_INLINE iwrc _fsm_del_fbk(FSM *impl, uint64_t offset_blk, uint64_t length_blk) {
  FSMBK fbk;
  assert(length_blk);
  iwrc rc = _fsm_init_fbk(&fbk, offset_blk, length_blk);
  RCRET(rc);
#ifndef NDEBUG
  int s2, s1 = kb_size(impl->fsm);
#endif
  kb_delp(fsm, impl->fsm, &fbk);
#ifndef NDEBUG
  s2 = kb_size(impl->fsm);
  assert(s2 < s1);
#endif
  if (FSMBK_OFFSET(&fbk) == impl->lfbkoff) {
    impl->lfbkoff = 0;
    impl->lfbklen = 0;
  }
  return 0;
}

/**
 * @brief Deregister free-block chunk from the fsm tree.
 * @param impl `FSM`
 * @param fbk `FSMBK` Fsm tree key structure.
 */
IW_INLINE void _fsm_del_fbk2(FSM *impl, const FSMBK fbk) {
#ifndef NDEBUG
  int s2, s1 = kb_size(impl->fsm);
#endif
  kb_delp(fsm, impl->fsm, &fbk);
#ifndef NDEBUG
  s2 = kb_size(impl->fsm);
  assert(s2 < s1);
#endif
  if (FSMBK_OFFSET(&fbk) == impl->lfbkoff) {
    impl->lfbkoff = 0;
    impl->lfbklen = 0;
  }
}

/**
 * @brief Register free space block in the fsm tree.
 * @param impl `FSM`
 * @param offset_blk Offset block number
 * @param length_blk Number of blocks
 */
IW_INLINE iwrc _fsm_put_fbk(FSM *impl, uint64_t offset_blk, uint64_t length_blk) {
  FSMBK fbk;
  assert(length_blk);
  iwrc rc = _fsm_init_fbk(&fbk, offset_blk, length_blk);
  RCRET(rc);
  kb_putp(fsm, impl->fsm, &fbk);
  if (offset_blk + length_blk >= impl->lfbkoff + impl->lfbklen) {
    impl->lfbkoff = offset_blk;
    impl->lfbklen = length_blk;
  }
  return 0;
}

/**
 * @brief Get the nearest free-space block.
 * @param impl `FSM`
 * @param offset_blk Desired offset in number of blocks.
 * @param length_blk Desired free area size specified in blocks.
 * @param opts Allocation opts
 * @return `0` if matching block is not found.
 */
IW_INLINE FSMBK *_fsm_find_matching_fblock_lw(FSM *impl,
                                              uint64_t offset_blk,
                                              uint64_t length_blk,
                                              iwfs_fsm_aflags opts) {
  FSMBK k, *uk, *lk;
  iwrc rc = _fsm_init_fbk(&k, offset_blk, length_blk);
  if (rc) {
    iwlog_ecode_error3(rc);
    return 0;
  }

  kb_intervalp(fsm, impl->fsm, &k, &lk, &uk);

  uint64_t lklength = lk ? FSMBK_LENGTH(lk) : 0;
  uint64_t uklength = uk ? FSMBK_LENGTH(uk) : 0;
  if (lklength == length_blk) {
    return lk;
  } else if (uklength == length_blk) {
    return uk;
  }
  if (lklength > uklength) {
    return (lklength > length_blk) ? lk : 0;
  } else {
    return (uklength > length_blk) ? uk : 0;
  }
}

/**
 * @brief Set the allocation bits in the fsm bitmap.
 *
 * @param impl
 * @param offset_bits Bit offset in the bitmap.
 * @param length_bits Number of bits to set
 * @param bit_status  If `1` bits will be set to `1` otherwise `0`
 * @param opts        Operation options
 */
static iwrc _fsm_set_bit_status_lw(FSM *impl,
                                   const uint64_t offset_bits,
                                   const uint64_t length_bits_,
                                   const int bit_status,
                                   const fsm_bmopts_t opts) {
  iwrc rc;
  size_t sp;
  uint8_t *mm;
  register int64_t length_bits = length_bits_;
  register uint64_t *p, set_mask;
  uint64_t bend = offset_bits + length_bits;
  int set_bits;

  if (bend < offset_bits) { // overflow
    return IW_ERROR_OUT_OF_BOUNDS;
  }
  assert(impl->bmlen * 8 >= offset_bits + length_bits);
  if (impl->bmlen * 8 < offset_bits + length_bits) {
    return IWFS_ERROR_FSM_SEGMENTATION;
  }
  if (impl->mmap_all) {
    rc = impl->pool.probe_mmap(&impl->pool, 0, &mm, &sp);
    RCRET(rc);
    if (sp < impl->bmoff + impl->bmlen) {
      return IWFS_ERROR_NOT_MMAPED;
    } else {
      mm += impl->bmoff;
    }
  } else {
    rc = impl->pool.probe_mmap(&impl->pool, impl->bmoff, &mm, &sp);
    RCRET(rc);
    if (sp < impl->bmlen) {
      return IWFS_ERROR_NOT_MMAPED;
    }
  }
  p = ((uint64_t *) mm) + offset_bits / 64;
  set_bits = 64 - (offset_bits & (64 - 1)); // NOLINT
  set_mask = (~((uint64_t) 0) << (offset_bits & (64 - 1)));

#ifdef IW_BIGENDIAN
  while (length_bits - set_bits >= 0) {
    uint64_t pv = *p;
    pv = IW_ITOHLL(pv);
    if (bit_status) {
      if ((opts & FSM_BM_STRICT) && (pv & set_mask)) {
        rc = IWFS_ERROR_FSM_SEGMENTATION;
      }
      if ((opts & FSM_BM_DRY_RUN) == 0) {
        pv |= set_mask;
        *p = IW_HTOILL(pv);
      }
    } else {
      if ((opts & FSM_BM_STRICT) && ((pv & set_mask) != set_mask)) {
        rc = IWFS_ERROR_FSM_SEGMENTATION;
      }
      if ((opts & FSM_BM_DRY_RUN) == 0) {
        pv &= ~set_mask;
        *p = IW_HTOILL(pv);
      }
    }
    length_bits -= set_bits;
    set_bits = 64;
    set_mask = ~((uint64_t) 0);
    ++p;
  }
  if (length_bits) {
    uint64_t pv = *p;
    pv = IW_ITOHLL(pv);
    set_mask &= (bend & (64 - 1)) ? ((((uint64_t) 1) << (bend & (64 - 1))) - 1) : ~((uint64_t) 0);
    if (bit_status) {
      if ((opts & FSM_BM_STRICT) && (pv & set_mask)) {
        rc = IWFS_ERROR_FSM_SEGMENTATION;
      }
      if ((opts & FSM_BM_DRY_RUN) == 0) {
        pv |= set_mask;
        *p = IW_HTOILL(pv);
      }
    } else {
      if ((opts & FSM_BM_STRICT) && (pv & set_mask) != set_mask) {
        rc = IWFS_ERROR_FSM_SEGMENTATION;
      }
      if ((opts & FSM_BM_DRY_RUN) == 0) {
        pv &= ~set_mask;
        *p = IW_HTOILL(pv);
      }
    }
  }
#else
  while (length_bits - set_bits >= 0) {
    if (bit_status) {
      if ((opts & FSM_BM_STRICT) && (*p & set_mask)) {
        rc = IWFS_ERROR_FSM_SEGMENTATION;
      }
      if ((opts & FSM_BM_DRY_RUN) == 0) {
        *p |= set_mask;
      }
    } else {
      if ((opts & FSM_BM_STRICT) && ((*p & set_mask) != set_mask)) {
        rc = IWFS_ERROR_FSM_SEGMENTATION;
      }
      if ((opts & FSM_BM_DRY_RUN) == 0) {
        *p &= ~set_mask;
      }
    }
    length_bits -= set_bits;
    set_bits = 64;
    set_mask = ~((uint64_t) 0);
    ++p;
  }
  if (length_bits) {
    set_mask &= (bend & (64 - 1)) ? ((((uint64_t) 1) << (bend & (64 - 1))) - 1) : ~((uint64_t) 0);
    if (bit_status) {
      if ((opts & FSM_BM_STRICT) && (*p & set_mask)) {
        rc = IWFS_ERROR_FSM_SEGMENTATION;
      }
      if ((opts & FSM_BM_DRY_RUN) == 0) {
        *p |= set_mask;
      }
    } else {
      if ((opts & FSM_BM_STRICT) && (*p & set_mask) != set_mask) {
        rc = IWFS_ERROR_FSM_SEGMENTATION;
      }
      if ((opts & FSM_BM_DRY_RUN) == 0) {
        *p &= ~set_mask;
      }
    }
  }
#endif
  if (!rc && impl->dlsnr) {
    uint64_t so = offset_bits / 8;
    uint64_t lb = length_bits_ + offset_bits % 8;
    uint64_t dl = lb / 8;
    if (lb % 8) ++dl;
    rc = impl->dlsnr->onwrite(impl->dlsnr, impl->bmoff + so, mm + so, dl, 0);
  }
  return rc;
}

/**
 *  @brief Allocate a continuous segment of blocks with page aligned offset.
 *
 *  @param impl `FSM`
 *  @param length_blk Desired segment length in blocks.
 *  @param [in,out] offset_blk Allocated segment offset in blocks will be stored into.
                    It also specified the desired segment offset to provide
 *                  allocation locality.
 *  @param [out] olength_blk Assigned segment length in blocks.
 *  @param  max_offset_blk Maximal offset of allocated block.
 *  @param opts Allocation options.
 */
static iwrc _fsm_blk_allocate_aligned_lw(FSM *impl,
                                         const uint64_t length_blk,
                                         uint64_t *offset_blk,
                                         uint64_t *olength_blk,
                                         const uint64_t max_offset_blk,
                                         const iwfs_fsm_aflags opts) {
  FSMBK *nk;
  fsm_bmopts_t bopts = FSM_BM_NONE;
  size_t aunit_blk = (impl->aunit >> impl->bpow);

  assert(impl && impl->fsm && length_blk > 0);
  if (impl->oflags & IWFSM_STRICT) {
    bopts |= FSM_BM_STRICT;
  }
  *olength_blk = 0;
  *offset_blk = 0;

  /* First attempt */
  nk = _fsm_find_matching_fblock_lw(impl, 0, length_blk + aunit_blk, opts);
  if (!nk) {
    nk = _fsm_find_matching_fblock_lw(impl, 0, length_blk, opts);
    if (!nk) {
      return IWFS_ERROR_NO_FREE_SPACE;
    }
  }
  uint64_t akoff = FSMBK_OFFSET(nk);
  uint64_t aklen = FSMBK_LENGTH(nk);
  uint64_t noff = IW_ROUNDUP(akoff, aunit_blk);

  if (noff <= max_offset_blk && (noff < aklen + akoff) && (aklen - (noff - akoff) >= length_blk)) {
    _fsm_del_fbk(impl, akoff, aklen);
    aklen = aklen - (noff - akoff);
    if (noff > akoff) {
      _fsm_put_fbk(impl, akoff, noff - akoff);
    }
    if (aklen > length_blk) {
      _fsm_put_fbk(impl, noff + length_blk, aklen - length_blk);
    }
    *offset_blk = noff;
    *olength_blk = length_blk;
    return _fsm_set_bit_status_lw(impl, noff, length_blk, 1, bopts);
  }

  aklen = 0;
  akoff = UINT64_MAX;
  /* full scan */
#define _fsm_traverse(k)                                                                                     \
  {                                                                                                          \
    uint64_t koff = FSMBK_OFFSET(k);                                                                         \
    uint64_t klen = FSMBK_LENGTH(k);                                                                         \
    if (koff < akoff) {                                                                                      \
      noff = IW_ROUNDUP(koff, aunit_blk);                                                                    \
      if (noff <= max_offset_blk && (noff < klen + koff) && (klen - (noff - koff) >= length_blk)) {          \
        akoff = koff;                                                                                        \
        aklen = klen;                                                                                        \
      }                                                                                                      \
    }                                                                                                        \
  }
  //-V:__kb_traverse:576, 701, 619, 769, 522
  __kb_traverse(FSMBK, impl->fsm, _fsm_traverse);
#undef _fsm_traverse

  if (akoff == UINT64_MAX) {
    return IWFS_ERROR_NO_FREE_SPACE;
  }
  _fsm_del_fbk(impl, akoff, aklen);
  noff = IW_ROUNDUP(akoff, aunit_blk);
  aklen = aklen - (noff - akoff);
  if (noff > akoff) {
    _fsm_put_fbk(impl, akoff, noff - akoff);
  }
  if (aklen > length_blk) {
    _fsm_put_fbk(impl, noff + length_blk, aklen - length_blk);
  }
  *offset_blk = noff;
  *olength_blk = length_blk;
  return _fsm_set_bit_status_lw(impl, noff, length_blk, 1, bopts);
}

/**
 * @brief Load existing bitmap area into free-space search tree.
 * @param impl  `FSM`
 * @param bm    Bitmap area start ptr
 * @param len   Bitmap area length in bytes.
 */
static void _fsm_load_fsm_lw(FSM *impl, const uint8_t *bm, uint64_t len) {
  uint64_t cbnum = 0, fbklength = 0, fbkoffset = 0;
  if (impl->fsm) {
    // -V:kb_destroy:701, 769
    kb_destroy(fsm, impl->fsm);
  }
  impl->fsm = kb_init(fsm, KB_DEFAULT_SIZE);
  for (uint64_t b = 0; b < len; ++b) {
    register uint8_t bb = bm[b];
    if (bb == 0) {
      fbklength += 8;
      cbnum += 8;
    } else if (bb == 0xffU) {
      if (fbklength) {
        fbkoffset = cbnum - fbklength;
        _fsm_put_fbk(impl, fbkoffset, fbklength);
        fbklength = 0;
      }
      cbnum += 8;
    } else {
      for (int i = 0; i < 8; ++i, ++cbnum) {
        if (bb & (1U << i)) {
          if (fbklength) {
            fbkoffset = cbnum - fbklength;
            _fsm_put_fbk(impl, fbkoffset, fbklength);
            fbklength = 0;
          }
        } else {
          ++fbklength;
        }
      }
    }
  }
  if (fbklength > 0) {
    fbkoffset = len * 8 - fbklength;
    _fsm_put_fbk(impl, fbkoffset, fbklength);
  }
}

/**
 * @brief Flush a current `iwfsmfile` metadata into the file header.
 * @param impl
 * @param is_sync If `1` perform mmap sync.
 * @return
 */
static iwrc _fsm_write_meta_lw(FSM *impl) {
  uint64_t llv;
  size_t wlen;
  uint32_t sp = 0, lv;
  uint8_t hdr[IWFSM_CUSTOM_HDR_DATA_OFFSET] = {0};

  /*
      [FSM_CTL_MAGICK u32][block pow u8]
      [bmoffset u64][bmlength u64]
      [u64 crzsum][u32 crznum][u64 crszvar][u256 reserved]
      [custom header size u32][custom header data...]
      [fsm data...]
  */

  /* magic */
  lv = IW_HTOIL(IWFSM_MAGICK);
  assert(sp + sizeof(lv) <= IWFSM_CUSTOM_HDR_DATA_OFFSET);
  memcpy(hdr + sp, &lv, sizeof(lv));
  sp += sizeof(lv);

  /* block pow */
  static_assert(sizeof(impl->bpow) == 1, "sizeof(impl->bpow) == 1");
  assert(sp + sizeof(impl->bpow) <= IWFSM_CUSTOM_HDR_DATA_OFFSET);
  memcpy(hdr + sp, &impl->bpow, sizeof(impl->bpow));
  sp += sizeof(impl->bpow);

  /* fsm bitmap block offset */
  llv = impl->bmoff;
  llv = IW_HTOILL(llv);
  assert(sp + sizeof(llv) <= IWFSM_CUSTOM_HDR_DATA_OFFSET);
  memcpy(hdr + sp, &llv, sizeof(llv));
  sp += sizeof(llv);

  /* fsm bitmap block length */
  llv = impl->bmlen;
  llv = IW_HTOILL(llv);
  assert(sp + sizeof(llv) <= IWFSM_CUSTOM_HDR_DATA_OFFSET);
  memcpy(hdr + sp, &llv, sizeof(llv));
  sp += sizeof(llv);

  /* Cumulative sum of record sizes acquired by `allocate` */
  llv = impl->crzsum;
  llv = IW_HTOILL(llv);
  assert(sp + sizeof(llv) <= IWFSM_CUSTOM_HDR_DATA_OFFSET);
  memcpy(hdr + sp, &llv, sizeof(llv));
  sp += sizeof(llv);

  /* Cumulative number of records acquired by `allocated` */
  lv = impl->crznum;
  lv = IW_HTOIL(lv);
  assert(sp + sizeof(lv) <= IWFSM_CUSTOM_HDR_DATA_OFFSET);
  memcpy(hdr + sp, &lv, sizeof(lv));
  sp += sizeof(lv);

  /* Record sizes standard variance (deviation^2 * N) */
  llv = impl->crzvar;
  llv = IW_HTOILL(llv);
  assert(sp + sizeof(lv) <= IWFSM_CUSTOM_HDR_DATA_OFFSET);
  memcpy(hdr + sp, &llv, sizeof(llv));
  sp += sizeof(llv);

  /* Reserved */
  sp += 32;

  /* Size of header */
  lv = impl->hdrlen;
  lv = IW_HTOIL(lv);
  assert(sp + sizeof(lv) <= IWFSM_CUSTOM_HDR_DATA_OFFSET);
  memcpy(hdr + sp, &lv, sizeof(lv));
  sp += sizeof(lv);

  assert(sp == IWFSM_CUSTOM_HDR_DATA_OFFSET);
  return impl->pool.write(&impl->pool, 0, hdr, IWFSM_CUSTOM_HDR_DATA_OFFSET, &wlen);
}

/**
 * @brief Search for the first next set bit position
 *        starting from the specified offset bit (INCLUDED).
 */
static uint64_t _fsm_find_next_set_bit(const uint64_t *addr,
                                       register uint64_t offset_bit,
                                       const uint64_t max_offset_bit,
                                       int *found) {
  *found = 0;
  register uint64_t bit, size;
  register const uint64_t *p = addr + offset_bit / 64;
  if (offset_bit >= max_offset_bit) {
    return 0;
  }
  bit = offset_bit & (64 - 1);
  offset_bit -= bit;
  size = max_offset_bit - offset_bit;

#ifdef IW_BIGENDIAN
  uint64_t pv = *p;
  if (bit) {
    pv = IW_ITOHLL(pv) & (~((uint64_t) 0) << bit);
    if (pv) {
      pv = iwbits_find_first_sbit64(pv);
      if (pv >= size) {
        return 0;
      } else {
        *found = 1;
        return offset_bit + pv;
      }
    }
    if (size <= 64) {
      return 0;
    }
    offset_bit += 64;
    size -= 64;
    ++p;
  }
  while (size & ~(64 - 1)) {
    pv = *(p++);
    if (pv) {
      *found = 1;
      return offset_bit + iwbits_find_first_sbit64(IW_ITOHLL(pv));
    }
    offset_bit += 64;
    size -= 64;
  }
  if (!size) {
    return 0;
  }
  pv = *p;
  pv = IW_ITOHLL(pv) & (~((uint64_t) 0) >> (64 - size));
  if (pv) {
    *found = 1;
    return offset_bit + iwbits_find_first_sbit64(pv);
  } else {
    return 0;
  }
#else
  register uint64_t tmp;
  if (bit) {
    tmp = *p & (~((uint64_t) 0) << bit);
    if (tmp) {
      tmp = iwbits_find_first_sbit64(tmp);
      if (tmp >= size) {
        return 0;
      } else {
        *found = 1;
        return offset_bit + tmp;
      }
    }
    if (size <= 64) {
      return 0;
    }
    offset_bit += 64;
    size -= 64;
    ++p;
  }
  while (size & ~(64 - 1)) {
    if ((tmp = *(p++))) {
      *found = 1;
      return offset_bit + iwbits_find_first_sbit64(tmp);
    }
    offset_bit += 64;
    size -= 64;
  }
  if (!size) {
    return 0;
  }
  tmp = (*p) & (~((uint64_t) 0) >> (64 - size));
  if (tmp) {
    *found = 1;
    return offset_bit + iwbits_find_first_sbit64(tmp);
  } else {
    return 0;
  }
#endif
}

/**
 * @brief Search for the first previous set bit position
 *        starting from the specified offset_bit (EXCLUDED).
 */
static uint64_t _fsm_find_prev_set_bit(const uint64_t *addr,
                                       register uint64_t offset_bit,
                                       const uint64_t min_offset_bit,
                                       int *found) {
  register const uint64_t *p;
  register uint64_t tmp, bit, size;
  *found = 0;
  if (min_offset_bit >= offset_bit) {
    return 0;
  }
  size = offset_bit - min_offset_bit;
  bit = offset_bit & (64 - 1);
  p = addr + offset_bit / 64;

#ifdef IW_BIGENDIAN
  uint64_t pv;
  if (bit) {
    pv = *p;
    pv = (iwbits_reverse_64(IW_ITOHLL(pv)) >> (64 - bit));
    if (pv) {
      pv = iwbits_find_first_sbit64(pv);
      if (pv >= size) {
        return 0;
      } else {
        *found = 1;
        assert(offset_bit > pv);
        return offset_bit > pv ? offset_bit - pv - 1 : 0;
      }
    }
    offset_bit -= bit;
    size -= bit;
  }
  while (size & ~(64 - 1)) {
    if (*(--p)) {
      pv = *p;
      *found = 1;
      tmp = iwbits_find_first_sbit64(iwbits_reverse_64(IW_ITOHLL(pv)));
      assert(offset_bit > tmp);
      return offset_bit > tmp ? offset_bit - tmp - 1 : 0;
    }
    offset_bit -= 64;
    size -= 64;
  }
  if (size == 0) {
    return 0;
  }
  pv = *(--p);
  tmp = iwbits_reverse_64(IW_ITOHLL(pv)) & ((((uint64_t) 1) << size) - 1);
#else
  if (bit) {
    tmp = (iwbits_reverse_64(*p) >> (64 - bit));
    if (tmp) {
      tmp = iwbits_find_first_sbit64(tmp);
      if (tmp >= size) {
        return 0;
      } else {
        *found = 1;
        assert(offset_bit > tmp);
        return offset_bit > tmp ? offset_bit - tmp - 1 : 0;
      }
    }
    offset_bit -= bit;
    size -= bit;
  }
  while (size & ~(64 - 1)) {
    if (*(--p)) {
      *found = 1;
      tmp = iwbits_find_first_sbit64(iwbits_reverse_64(*p));
      assert(offset_bit > tmp);
      return offset_bit > tmp ? offset_bit - tmp - 1 : 0;
    }
    offset_bit -= 64;
    size -= 64;
  }
  if (size == 0) {
    return 0;
  }
  tmp = iwbits_reverse_64(*(--p)) & ((((uint64_t) 1) << size) - 1);
#endif
  if (tmp) {
    uint64_t tmp2;
    *found = 1;
    tmp2 = iwbits_find_first_sbit64(tmp);
    assert(offset_bit > tmp2);
    return offset_bit > tmp2 ? offset_bit - tmp2 - 1 : 0;
  } else {
    return 0;
  }
}

/**
 * @brief Return a previously allocated blocks
 *        back into the free-blocks pool.
 *
 * @param impl `FSM`
 * @param offset_blk Starting block number of the specified range.
 * @param length_blk Range size in blocks.
 */
static iwrc _fsm_blk_deallocate_lw(FSM *impl,
                                   const uint64_t offset_blk,
                                   const uint64_t length_blk) {
  iwrc rc;
  uint64_t *bmptr;
  uint64_t left, right;
  int hasleft = 0, hasright = 0;
  uint64_t key_offset = offset_blk, key_length = length_blk;
  uint64_t rm_offset = 0, rm_length = 0;
  uint64_t lfbkoff = impl->lfbkoff;
  uint64_t end_offset_blk = offset_blk + length_blk;
  fsm_bmopts_t bopts = FSM_BM_NONE;


  if (impl->oflags & IWFSM_STRICT) {
    bopts |= FSM_BM_STRICT;
  }
  rc = _fsm_set_bit_status_lw(impl, offset_blk, length_blk, 0, bopts);
  RCRET(rc);

  rc = _fsm_bmptr(impl, &bmptr);
  RCRET(rc);

  /* Merge with neighborhoods */
  left = _fsm_find_prev_set_bit(bmptr, offset_blk, 0, &hasleft);
  if (lfbkoff && lfbkoff == end_offset_blk) {
    right = lfbkoff + impl->lfbklen;
    hasright = 1;
  } else {
    uint64_t maxoff = lfbkoff ? lfbkoff : (impl->bmlen << 3);
    right = _fsm_find_next_set_bit(bmptr, end_offset_blk, maxoff, &hasright);
  }

  if (hasleft) {
    if (offset_blk > left + 1) {
      left += 1;
      rm_offset = left;
      rm_length = offset_blk - left;
      IWRC(_fsm_del_fbk(impl, rm_offset, rm_length), rc);
      key_offset = rm_offset;
      key_length += rm_length;
    }
  } else if (offset_blk > 0) { /* zero start */
    rm_offset = 0;
    rm_length = offset_blk;
    IWRC(_fsm_del_fbk(impl, rm_offset, rm_length), rc);
    key_offset = rm_offset;
    key_length += rm_length;
  }
  if (hasright && right > end_offset_blk) {
    rm_offset = end_offset_blk;
    rm_length = right - end_offset_blk;
    _fsm_del_fbk(impl, rm_offset, rm_length);
    key_length += rm_length;
  }
  IWRC(_fsm_put_fbk(impl, key_offset, key_length), rc);
  return rc;
}

/**
 * @brief Initialize a new free-space bitmap area.
 *
 * If bitmap exists, its content will be moved into newly created area.
 * Blocks from the previous bitmap are will disposed and deallocated.
 *
 * @param impl `FSM`
 * @param bmoff Byte offset of the new bitmap. Value must be page aligned.
 * @param bmlen Byte length of the new bitmap. Value must be page aligned.
                Its length must not be lesser than length of old bitmap.
 */
static iwrc _fsm_init_lw(FSM *impl, uint64_t bmoff, uint64_t bmlen) {
  iwrc rc;
  uint8_t *mm, *mm2;
  size_t sp, sp2;
  uint64_t old_bmoff, old_bmlen;
  IWFS_EXT *pool = &impl->pool;

  if ((bmlen & ((1U << impl->bpow) - 1)) || (bmoff & ((1U << impl->bpow) - 1)) || (bmoff & (impl->aunit - 1))) {
    return IWFS_ERROR_RANGE_NOT_ALIGNED;
  }
  if (bmlen < impl->bmlen) {
    rc = IW_ERROR_INVALID_ARGS;
    iwlog_ecode_error(rc, "Length of the newly initiated bitmap area (bmlen): %" PRIu64
                      " must not be lesser than the current bitmap area length %" PRIu64 "",
                      bmlen, impl->bmlen);
    return rc;
  }
  if (bmlen * 8 < ((bmoff + bmlen) >> impl->bpow) + 1) {
    rc = IW_ERROR_INVALID_ARGS;
    iwlog_ecode_error(rc, "Length of the newly initiated bitmap area (bmlen): %" PRIu64
                      " is not enough to handle bitmap itself and the file header area.",
                      bmlen);
    return rc;
  }
  rc = _fsm_ensure_size_lw(impl, bmoff + bmlen);
  RCRET(rc);
  if (impl->mmap_all) {
    // get mmap area without locking, since we ensured what pool file will not be remapped
    rc = pool->probe_mmap(pool, 0, &mm, &sp);
    RCRET(rc);
    if (sp < bmoff + bmlen) {
      return IWFS_ERROR_NOT_MMAPED;
    } else {
      mm += bmoff;
    }
  } else {
    // get mmap area without locking, since we ensured what pool file will not be remapped
    rc = pool->probe_mmap(pool, bmoff, &mm, &sp);
    RCRET(rc);
    if (sp < bmlen) {
      return IWFS_ERROR_NOT_MMAPED;
    }
  }
  if (impl->bmlen) {
    /* We have an old active bitmap. Lets copy its content to the new location.*/
    if (IW_RANGES_OVERLAP(impl->bmoff, impl->bmoff + impl->bmlen, bmoff, bmoff + bmlen)) {
      iwlog_ecode_error2(rc, "New and old bitmap areas are overlaped");
      return IW_ERROR_INVALID_ARGS;
    }
    if (impl->mmap_all) {
      mm2 = mm - bmoff + impl->bmoff;
    } else {
      rc = pool->probe_mmap(pool, impl->bmoff, &mm2, &sp2);
      if (!rc && sp2 < impl->bmlen) {
        rc = IWFS_ERROR_NOT_MMAPED;
      }
      if (rc) {
        iwlog_ecode_error2(rc, "Old bitmap area is not mmaped");
        return rc;
      }
    }
    assert(!((impl->bmlen - bmlen) & ((1U << impl->bpow) - 1)));
    if (impl->dlsnr) {
      rc = impl->dlsnr->onwrite(impl->dlsnr, bmoff, mm2, impl->bmlen, 0);
      RCRET(rc);
    }
    memcpy(mm, mm2, impl->bmlen);
    if (bmlen > impl->bmlen) {
      memset(mm + impl->bmlen, 0, bmlen - impl->bmlen);
      if (impl->dlsnr) {
        rc = impl->dlsnr->onset(impl->dlsnr, bmoff + impl->bmlen, 0, bmlen - impl->bmlen, 0);
        RCRET(rc);
      }
    }
  } else {
    mm2 = 0;
    memset(mm, 0, bmlen);
    if (impl->dlsnr) {
      rc = impl->dlsnr->onset(impl->dlsnr, bmoff, 0, bmlen, 0);
      RCRET(rc);
    }
  }

  /* Backup the previous bitmap range */
  old_bmlen = impl->bmlen;
  old_bmoff = impl->bmoff;
  impl->bmoff = bmoff;
  impl->bmlen = bmlen;

  rc = _fsm_set_bit_status_lw(impl, (bmoff >> impl->bpow), (bmlen >> impl->bpow), 1, FSM_BM_NONE);
  RCGO(rc, rollback);
  if (!old_bmlen) { /* First time initialization */
    /* Header allocation */
    rc = _fsm_set_bit_status_lw(impl, 0, (impl->hdrlen >> impl->bpow), 1, FSM_BM_NONE);
    RCGO(rc, rollback);
  }

  /* Reload fsm tree */
  _fsm_load_fsm_lw(impl, mm, bmlen);

  /* Flush new meta */
  rc = _fsm_write_meta_lw(impl);
  RCGO(rc, rollback);

  rc = pool->sync(pool, IWFS_FDATASYNC);
  RCGO(rc, rollback);

  if (old_bmlen) {
    /* Now we are save to deallocate the old bitmap */
    rc = _fsm_blk_deallocate_lw(impl, (old_bmoff >> impl->bpow), (old_bmlen >> impl->bpow));
    if (!impl->mmap_all) {
      pool->remove_mmap(pool, old_bmoff);
    }
  }
  return rc;

rollback: /* try to rollback previous bitmap state */
  impl->bmoff = old_bmoff;
  impl->bmlen = old_bmlen;
  if (old_bmlen && mm2) {
    _fsm_load_fsm_lw(impl, mm2, old_bmlen);
  }
  pool->sync(pool, IWFS_FDATASYNC);
  return rc;
}

/**
 * @brief Resize bitmap area.
 * @param impl `FSM`
 * @param size New size of bitmap area in bytes.
 */
static iwrc _fsm_resize_fsm_bitmap_lw(FSM *impl, uint64_t size) {
  iwrc rc;
  uint64_t bmoffset = 0, bmlen, sp;
  IWFS_EXT *pool = &impl->pool;

  if (impl->bmlen >= size) {
    return 0;
  }
  bmlen = IW_ROUNDUP(size, impl->aunit); /* align to the system page size. */
  rc = _fsm_blk_allocate_aligned_lw(
         impl, (bmlen >> impl->bpow), &bmoffset, &sp, UINT64_MAX,
         IWFSM_ALLOC_NO_STATS | IWFSM_ALLOC_NO_EXTEND | IWFSM_ALLOC_NO_OVERALLOCATE);
  if (!rc) {
    bmoffset = bmoffset << impl->bpow;
    bmlen = sp << impl->bpow;
  } else if (rc == IWFS_ERROR_NO_FREE_SPACE) {
    bmoffset = impl->bmlen * (1 << impl->bpow) * 8;
    bmoffset = IW_ROUNDUP(bmoffset, impl->aunit);
  }
  if (!impl->mmap_all) {
    rc = pool->add_mmap(pool, bmoffset, bmlen, impl->mmap_opts);
    RCRET(rc);
  }
  rc = _fsm_init_lw(impl, bmoffset, bmlen);
  if (rc && !impl->mmap_all) {
    pool->remove_mmap(pool, bmoffset);
  }
  return rc;
}

/**
 * @brief Allocate a continuous segment of blocks.
 *
 * @param impl `FSM`
 * @param length_blk Desired segment length in blocks.
 * @param [in,out] offset_blk Allocated segment offset in blocks will be stored into.
 *                It also specified the desired segment offset to provide allocation locality.
 * @param [out] olength_blk Assigned segment length in blocks.
 * @param opts
 */
static iwrc _fsm_blk_allocate_lw(FSM *impl,
                                 uint64_t length_blk,
                                 uint64_t *offset_blk,
                                 uint64_t *olength_blk,
                                 iwfs_fsm_aflags opts) {
  iwrc rc;
  FSMBK *nk;
  fsm_bmopts_t bopts = FSM_BM_NONE;

  if (opts & IWFSM_ALLOC_PAGE_ALIGNED) {
    while (1) {
      rc = _fsm_blk_allocate_aligned_lw(impl, length_blk, offset_blk, olength_blk, UINT64_MAX, opts);
      if (rc == IWFS_ERROR_NO_FREE_SPACE) {
        if (opts & IWFSM_ALLOC_NO_EXTEND) {
          return IWFS_ERROR_NO_FREE_SPACE;
        }
        rc = _fsm_resize_fsm_bitmap_lw(impl, impl->bmlen << 1);
        RCRET(rc);
        continue;
      }
      if (!rc && (opts & IWFSM_SOLID_ALLOCATED_SPACE)) {
        uint64_t bs = *offset_blk;
        int64_t bl = *olength_blk;
        rc = _fsm_ensure_size_lw(impl, (bs << impl->bpow) + (bl << impl->bpow));
      }
      return rc;
    }
  }

  *olength_blk = length_blk;

start:
  nk = _fsm_find_matching_fblock_lw(impl, *offset_blk, length_blk, opts);
  if (nk) { /* use existing free space block */
    uint64_t nlength = FSMBK_LENGTH(nk);
    *offset_blk = FSMBK_OFFSET(nk);
    assert(kb_get(fsm, impl->fsm, *nk));

    _fsm_del_fbk2(impl, *nk);

    if (nlength > length_blk) { /* re-save rest of free-space */
      if (!(opts & IWFSM_ALLOC_NO_OVERALLOCATE) && impl->crznum) {
        /* todo use lognormal distribution? */
        double_t d = ((double_t) impl->crzsum / (double_t) impl->crznum) /*avg*/
                     - (nlength - length_blk);                           /*rest blk size*/
        double_t s = ((double_t) impl->crzvar / (double_t) impl->crznum) * 6.0; /* blk size dispersion * 6 */
        if (s > 1 && d > 0 && d * d > s) { /* its better to attach rest of block to the record */
          *olength_blk = nlength;
        } else {
          _fsm_put_fbk(impl, (*offset_blk + length_blk), (nlength - length_blk));
        }
      } else {
        _fsm_put_fbk(impl, (*offset_blk + length_blk), (nlength - length_blk));
      }
    }
  } else {
    if (opts & IWFSM_ALLOC_NO_EXTEND) {
      return IWFS_ERROR_NO_FREE_SPACE;
    }
    rc = _fsm_resize_fsm_bitmap_lw(impl, impl->bmlen << 1);
    RCRET(rc);
    goto start;
  }

  if (impl->oflags & IWFSM_STRICT) {
    bopts |= FSM_BM_STRICT;
  }
  rc = _fsm_set_bit_status_lw(impl, *offset_blk, *olength_blk, 1, bopts);
  if (!rc && !(opts & IWFSM_ALLOC_NO_STATS)) {
    double_t avg;
    /* Update allocation statistics */
    if (impl->crznum > FSM_MAX_STATS_COUNT) {
      impl->crznum = 0;
      impl->crzsum = 0;
      impl->crzvar = 0;
    }
    ++impl->crznum;
    impl->crzsum += length_blk;
    avg = (double_t) impl->crzsum / (double_t) impl->crznum; /* average */
    impl->crzvar +=
      (uint64_t)(((double_t) length_blk - avg) * ((double_t) length_blk - avg) + 0.5L); /* variance */
  }
  if (!rc && (opts & IWFSM_SOLID_ALLOCATED_SPACE)) {
    uint64_t bs = *offset_blk;
    int64_t bl = *olength_blk;
    rc = _fsm_ensure_size_lw(impl, (bs << impl->bpow) + (bl << impl->bpow));
  }
  if (!rc && (opts & IWFSM_SYNC_BMAP)) {
    uint64_t *bmptr;
    if (!_fsm_bmptr(impl, &bmptr)) {
      IWFS_EXT *pool = &impl->pool;
      rc = pool->sync_mmap(pool, impl->bmoff, IWFS_SYNCDEFAULT);
    }
  }
  return rc;
}

/**
 * @brief Remove all free blocks from the and of file and trim its size.
 */
static iwrc _fsm_trim_tail_lw(FSM *impl) {
  iwrc rc;
  int hasleft;
  uint64_t length, lastblk, *bmptr;
  IWFS_EXT_STATE fstate;
  uint64_t offset = 0;

  if (!(impl->omode & IWFS_OWRITE)) {
    return 0;
  }
  /* find free space for fsm with lesser offset than actual */
  rc = _fsm_blk_allocate_aligned_lw(
         impl, (impl->bmlen >> impl->bpow), &offset, &length, (impl->bmoff >> impl->bpow),
         IWFSM_ALLOC_NO_EXTEND | IWFSM_ALLOC_NO_OVERALLOCATE | IWFSM_ALLOC_NO_STATS);

  if (rc && rc != IWFS_ERROR_NO_FREE_SPACE) {
    return rc;
  }
  if (rc) {
    rc = 0;
  } else if ((offset << impl->bpow) < impl->bmoff) {
    offset = offset << impl->bpow;
    length = length << impl->bpow;
    assert(offset != impl->bmoff);
    impl->pool.add_mmap(&impl->pool, offset, length, impl->mmap_opts);
    rc = _fsm_init_lw(impl, offset, length);
    RCGO(rc, finish);
  } else {
    /* shoud never be reached */
    assert(0);
    rc = _fsm_blk_deallocate_lw(impl, offset, length);
    RCGO(rc, finish);
  }

  rc = _fsm_bmptr(impl, &bmptr); // -V519
  RCGO(rc, finish);

  lastblk = (impl->bmoff + impl->bmlen) >> impl->bpow;
  offset = _fsm_find_prev_set_bit(bmptr, (impl->bmlen << 3), lastblk, &hasleft);
  if (hasleft) {
    lastblk = offset + 1;
  }
  rc = impl->pool.state(&impl->pool, &fstate);
  if (!rc && fstate.fsize > (lastblk << impl->bpow)) {
    rc = impl->pool.truncate(&impl->pool, lastblk << impl->bpow);
  }

finish:
  return rc;
}

static iwrc _fsm_init_impl(FSM *impl, const IWFS_FSM_OPTS *opts) {
  impl->oflags = opts->oflags;
  impl->aunit = iwp_alloc_unit();
  impl->bpow = opts->bpow;
  impl->mmap_all = opts->mmap_all;
  if (!impl->bpow) {
    impl->bpow = 6;  // 64bit block
  } else if (impl->bpow > FSM_MAX_BLOCK_POW) {
    return IWFS_ERROR_INVALID_BLOCK_SIZE;
  } else if ((1U << impl->bpow) > impl->aunit) {
    return IWFS_ERROR_PLATFORM_PAGE;
  }
  return 0;
}

static iwrc _fsm_init_locks(FSM *impl, const IWFS_FSM_OPTS *opts) {
  if (opts->oflags & IWFSM_NOLOCKS) {
    impl->ctlrwlk = 0;
    return 0;
  }
  impl->ctlrwlk = calloc(1, sizeof(*impl->ctlrwlk));
  if (!impl->ctlrwlk) {
    return iwrc_set_errno(IW_ERROR_ALLOC, errno);
  }
  int rci = pthread_rwlock_init(impl->ctlrwlk, 0);
  if (rci) {
    free(impl->ctlrwlk);
    impl->ctlrwlk = 0;
    return iwrc_set_errno(IW_ERROR_THREADING_ERRNO, rci);
  }
  return 0;
}

static iwrc _fsm_destroy_locks(FSM *impl) {
  if (!impl->ctlrwlk) {
    return 0;
  }
  iwrc rc = 0;
  int rci = pthread_rwlock_destroy(impl->ctlrwlk);
  if (rci) {
    IWRC(iwrc_set_errno(IW_ERROR_THREADING_ERRNO, rci), rc);
  }
  free(impl->ctlrwlk);
  impl->ctlrwlk = 0;
  return rc;
}

static iwrc _fsm_read_meta_lr(FSM *impl) {
  iwrc rc;
  uint32_t lv;
  uint64_t llv;
  size_t sp, rp = 0;
  uint8_t hdr[IWFSM_CUSTOM_HDR_DATA_OFFSET] = {0};

  /*
      [FSM_CTL_MAGICK u32][block pow u8]
      [bmoffset u64][bmlength u64]
      [u64 crzsum][u32 crznum][u64 crszvar][u256 reserved]
      [custom header size u32][custom header data...]
      [fsm data...]
  */

  rc = impl->pool.read(&impl->pool, 0, hdr, IWFSM_CUSTOM_HDR_DATA_OFFSET, &sp);
  if (rc) {
    iwlog_ecode_error3(rc);
    return rc;
  }

  /* Magic */
  memcpy(&lv, hdr + rp, sizeof(lv)); // -V512
  lv = IW_ITOHL(lv);
  if (lv != IWFSM_MAGICK) {
    rc = IWFS_ERROR_INVALID_FILEMETA;
    iwlog_ecode_error2(rc, "Invalid file magic number");
    return rc;
  }
  rp += sizeof(lv);

  /* Block pow */
  memcpy(&impl->bpow, hdr + rp, sizeof(impl->bpow));
  rp += sizeof(impl->bpow);

  if (impl->bpow > FSM_MAX_BLOCK_POW) {
    rc = IWFS_ERROR_INVALID_FILEMETA;
    iwlog_ecode_error(rc, "Invalid file blocks pow: %u", impl->bpow);
    return rc;
  }
  if ((1U << impl->bpow) > impl->aunit) {
    rc = IWFS_ERROR_PLATFORM_PAGE;
    iwlog_ecode_error(rc, "Block size: %u must not be greater than system page size: %zu",
                      (1U << impl->bpow), impl->aunit);
  }

  /* Free-space bitmap offset */
  memcpy(&llv, hdr + rp, sizeof(llv));
  llv = IW_ITOHLL(llv);
  impl->bmoff = llv;
  rp += sizeof(llv);

  /* Free-space bitmap length */
  memcpy(&llv, hdr + rp, sizeof(llv));
  llv = IW_ITOHLL(llv);
  impl->bmlen = llv;
  if (llv & (64 - 1)) {
    rc = IWFS_ERROR_INVALID_FILEMETA;
    iwlog_ecode_error(rc, "Free-space bitmap length is not 64bit aligned: %" PRIuMAX "", impl->bmlen);
  }
  rp += sizeof(llv);

  /* Cumulative sum of record sizes acquired by `allocate` */
  memcpy(&llv, hdr + rp, sizeof(llv));
  llv = IW_ITOHLL(llv);
  impl->crzsum = llv;
  rp += sizeof(llv);

  /* Cumulative number of records acquired by `allocated` */
  memcpy(&lv, hdr + rp, sizeof(lv));
  lv = IW_ITOHL(lv);
  impl->crznum = lv;
  rp += sizeof(lv);

  /* Record sizes standard variance (deviation^2 * N) */
  memcpy(&llv, hdr + rp, sizeof(llv));
  llv = IW_ITOHLL(llv);
  impl->crzvar = llv;
  rp += sizeof(llv);

  /* Reserved */
  rp += 32;

  /* Header size */
  memcpy(&lv, hdr + rp, sizeof(lv));
  lv = IW_ITOHL(lv);
  impl->hdrlen = lv;
  rp += sizeof(lv);

  assert(rp == IWFSM_CUSTOM_HDR_DATA_OFFSET);
  return rc;
}

static iwrc _fsm_init_new_lw(FSM *impl, const IWFS_FSM_OPTS *opts) {
  FSM_ENSURE_OPEN(impl);
  iwrc rc;
  uint64_t bmlen, bmoff;
  IWFS_EXT *pool = &impl->pool;
  assert(impl->aunit && impl->bpow);

  impl->hdrlen = opts->hdrlen + IWFSM_CUSTOM_HDR_DATA_OFFSET;
  impl->hdrlen = IW_ROUNDUP(impl->hdrlen, 1ULL << impl->bpow);
  bmlen = opts->bmlen > 0 ? IW_ROUNDUP(opts->bmlen, impl->aunit) : impl->aunit;
  bmoff = IW_ROUNDUP(impl->hdrlen, impl->aunit);

  if (impl->mmap_all) {
    /* mmap whole file */
    rc = pool->add_mmap(pool, 0, SIZE_T_MAX, impl->mmap_opts);
    RCRET(rc);
  } else {
    /* mmap header */
    rc = pool->add_mmap(pool, 0, impl->hdrlen, impl->mmap_opts);
    RCRET(rc);
    /* mmap the fsm bitmap index */
    rc = pool->add_mmap(pool, bmoff, bmlen, impl->mmap_opts);
    RCRET(rc);
  }
  return _fsm_init_lw(impl, bmoff, bmlen);
}

static iwrc _fsm_init_existing_lw(FSM *impl) {
  FSM_ENSURE_OPEN(impl);
  iwrc rc;
  size_t sp;
  uint8_t *mm;
  IWFS_EXT *pool = &impl->pool;

  rc = _fsm_read_meta_lr(impl);
  RCGO(rc, finish);

  if (impl->mmap_all) {
    /* mmap the whole file */
    rc = pool->add_mmap(pool, 0, SIZE_T_MAX, impl->mmap_opts);
    RCGO(rc, finish);
    rc = pool->probe_mmap(pool, 0, &mm, &sp);
    RCGO(rc, finish);
    if (sp < impl->bmoff + impl->bmlen) {
      rc = IWFS_ERROR_NOT_MMAPED;
      goto finish;
    } else {
      mm += impl->bmoff;
    }
  } else {
    /* mmap the header of file */
    rc = pool->add_mmap(pool, 0, impl->hdrlen, impl->mmap_opts);
    RCGO(rc, finish);
    /* mmap the fsm bitmap index */
    rc = pool->add_mmap(pool, impl->bmoff, impl->bmlen, impl->mmap_opts);
    RCGO(rc, finish);
    rc = pool->probe_mmap(pool, impl->bmoff, &mm, &sp);
    RCGO(rc, finish);
    if (sp < impl->bmlen) {
      rc = IWFS_ERROR_NOT_MMAPED;
      goto finish;
    }
  }

  _fsm_load_fsm_lw(impl, mm, impl->bmlen);

finish:
  return rc;
}

/**
 * @brief Check if all blocks within the specified range have been `allocated`.
 *
 * @param impl `FSM`
 * @param offset_blk Starting block number of the specified range.
 * @param length_blk Range size in blocks.
 * @param [out] ret Checking result.
 */
static iwrc _fsm_is_fully_allocated_lr(FSM *impl, uint64_t offset_blk, uint64_t length_blk, int *ret) {
  uint64_t end = offset_blk + length_blk;
  *ret = 1;
  if (length_blk < 1 || end < offset_blk || end > (impl->bmlen << 3)) {
    *ret = 0;
    return 0;
  }
  iwrc rc = _fsm_set_bit_status_lw(impl, offset_blk, length_blk, 0, FSM_BM_DRY_RUN | FSM_BM_STRICT);
  if (rc == IWFS_ERROR_FSM_SEGMENTATION) {
    *ret = 0;
    return 0;
  }
  return rc;
}

/*************************************************************************************************
 *                                  Public API *
 *************************************************************************************************/

static iwrc _fsm_write(struct IWFS_FSM *f, off_t off, const void *buf, size_t siz, size_t *sp) {
  FSM_ENSURE_OPEN2(f);
  FSM *impl = f->impl;
  iwrc rc = _fsm_ctrl_rlock(impl);
  RCRET(rc);
  if (impl->oflags & IWFSM_STRICT) {
    int allocated = 0;
    IWRC(_fsm_is_fully_allocated_lr(impl,
                                    (uint64_t) off >> impl->bpow,
                                    IW_ROUNDUP(siz, 1ULL << impl->bpow) >> impl->bpow,
                                    &allocated), rc);
    if (!rc) {
      if (!allocated) {
        rc = IWFS_ERROR_FSM_SEGMENTATION;
      } else {
        rc = impl->pool.write(&impl->pool, off, buf, siz, sp);
      }
    }
  } else {
    rc = impl->pool.write(&impl->pool, off, buf, siz, sp);
  }
  _fsm_ctrl_unlock(impl);
  return rc;
}

static iwrc _fsm_read(struct IWFS_FSM *f, off_t off, void *buf, size_t siz, size_t *sp) {
  FSM_ENSURE_OPEN2(f);
  FSM *impl = f->impl;
  iwrc rc = _fsm_ctrl_rlock(impl);
  RCRET(rc);
  if (impl->oflags & IWFSM_STRICT) {
    int allocated = 0;
    IWRC(_fsm_is_fully_allocated_lr(impl, (uint64_t) off >> impl->bpow,
                                    IW_ROUNDUP(siz, 1ULL << impl->bpow) >> impl->bpow,
                                    &allocated), rc);
    if (!rc) {
      if (!allocated) {
        rc = IWFS_ERROR_FSM_SEGMENTATION;
      } else {
        rc = impl->pool.read(&impl->pool, off, buf, siz, sp);
      }
    }
  } else {
    rc = impl->pool.read(&impl->pool, off, buf, siz, sp);
  }
  _fsm_ctrl_unlock(impl);
  return rc;
}

static iwrc _fsm_sync(struct IWFS_FSM *f, iwfs_sync_flags flags) {
  FSM_ENSURE_OPEN2(f);
  iwrc rc = _fsm_ctrl_rlock(f->impl);
  RCRET(rc);
  IWRC(_fsm_write_meta_lw(f->impl), rc);
  IWRC(f->impl->pool.sync(&f->impl->pool, flags), rc);
  IWRC(_fsm_ctrl_unlock(f->impl), rc);
  return rc;
}

static iwrc _fsm_close(struct IWFS_FSM *f) {
  if (!f || !f->impl) {
    return 0;
  }
  FSM *impl = f->impl;
  iwrc rc = 0;
  IWRC(_fsm_ctrl_wlock(impl), rc);
  if (impl->fsm && (impl->omode & IWFS_OWRITE)) {
    if (!(impl->oflags & IWFSM_NO_TRIM_ON_CLOSE)) {
      IWRC(_fsm_trim_tail_lw(impl), rc);
    }
    IWRC(_fsm_write_meta_lw(impl), rc);
    if (!impl->dlsnr) {
      IWRC(impl->pool.sync(&impl->pool, IWFS_SYNCDEFAULT), rc);
    }
  }
  IWRC(impl->pool.close(&impl->pool), rc);
  if (impl->fsm) {
    // -V:__kb_destroy:701, 769
    __kb_destroy(impl->fsm);
  }
  IWRC(_fsm_ctrl_unlock(impl), rc);
  IWRC(_fsm_destroy_locks(impl), rc);
  impl->f->impl = 0;
  impl->f = 0;
  free(impl);
  return rc;
}

IW_INLINE iwrc _fsm_ensure_size_lw(FSM *impl, off_t size) {
  return impl->pool.ensure_size(&impl->pool, size);
}

static iwrc _fsm_ensure_size(struct IWFS_FSM *f, off_t size) {
  FSM_ENSURE_OPEN2(f);
  iwrc rc = _fsm_ctrl_rlock(f->impl);
  RCRET(rc);
  if (f->impl->bmoff + f->impl->bmlen > size) {
    rc = IWFS_ERROR_RESIZE_FAIL;
    goto finish;
  }
  rc = _fsm_ensure_size_lw(f->impl, size);

finish:
  IWRC(_fsm_ctrl_unlock(f->impl), rc);
  return rc;
}

static iwrc _fsm_add_mmap(struct IWFS_FSM *f, off_t off, size_t maxlen, iwfs_ext_mmap_opts_t opts) {
  FSM_ENSURE_OPEN2(f);
  return f->impl->pool.add_mmap(&f->impl->pool, off, maxlen, opts);
}

static iwrc _fsm_remap_all(struct IWFS_FSM *f) {
  FSM_ENSURE_OPEN2(f);
  return f->impl->pool.remap_all(&f->impl->pool);
}

iwrc _fsm_acquire_mmap(struct IWFS_FSM *f, off_t off, uint8_t **mm, size_t *sp) {
  return f->impl->pool.acquire_mmap(&f->impl->pool, off, mm, sp);
}

iwrc _fsm_release_mmap(struct IWFS_FSM *f) {
  return f->impl->pool.release_mmap(&f->impl->pool);
}

static iwrc _fsm_probe_mmap(struct IWFS_FSM *f, off_t off, uint8_t **mm, size_t *sp) {
  FSM_ENSURE_OPEN2(f);
  return f->impl->pool.probe_mmap(&f->impl->pool, off, mm, sp);
}

static iwrc _fsm_remove_mmap(struct IWFS_FSM *f, off_t off) {
  FSM_ENSURE_OPEN2(f);
  return f->impl->pool.remove_mmap(&f->impl->pool, off);
}

static iwrc _fsm_sync_mmap(struct IWFS_FSM *f, off_t off, iwfs_sync_flags flags) {
  FSM_ENSURE_OPEN2(f);
  return f->impl->pool.sync_mmap(&f->impl->pool, off, flags);
}

static iwrc _fsm_allocate(struct IWFS_FSM *f, off_t len, off_t *oaddr, off_t *olen, iwfs_fsm_aflags opts) {
  FSM_ENSURE_OPEN2(f);
  iwrc rc;
  uint64_t sbnum, nlen;
  FSM *impl = f->impl;

  *olen = 0;
  if (!(impl->omode & IWFS_OWRITE)) {
    return IW_ERROR_READONLY;
  }
  if (len <= 0) {
    return IW_ERROR_INVALID_ARGS;
  }
  /* Required blocks number */
  sbnum = (uint64_t) * oaddr >> impl->bpow;
  len = IW_ROUNDUP(len, 1ULL << impl->bpow);

  rc = _fsm_ctrl_wlock(impl);
  RCRET(rc);
  rc = _fsm_blk_allocate_lw(f->impl, (uint64_t) len >> impl->bpow, &sbnum, &nlen, opts);
  if (!rc) {
    *olen = (nlen << impl->bpow);
    *oaddr = (sbnum << impl->bpow);
  }
  IWRC(_fsm_ctrl_unlock(impl), rc);
  return rc;
}

static iwrc _fsm_reallocate(struct IWFS_FSM *f, off_t nlen, off_t *oaddr, off_t *olen, iwfs_fsm_aflags opts) {
  FSM_ENSURE_OPEN2(f);
  iwrc rc;
  FSM *impl = f->impl;

  if (!(impl->omode & IWFS_OWRITE)) {
    return IW_ERROR_READONLY;
  }
  if ((*oaddr & ((1ULL << impl->bpow) - 1)) || (*olen & ((1ULL << impl->bpow) - 1))) {
    return IWFS_ERROR_RANGE_NOT_ALIGNED;
  }
  uint64_t sp;
  uint64_t nlen_blk = IW_ROUNDUP((uint64_t) nlen, 1ULL << impl->bpow) >> impl->bpow;
  uint64_t olen_blk = (uint64_t) * olen >> impl->bpow;
  uint64_t oaddr_blk = (uint64_t) * oaddr >> impl->bpow;
  uint64_t naddr_blk = oaddr_blk;

  if (nlen_blk == olen_blk) {
    return 0;
  }
  rc = _fsm_ctrl_wlock(impl);
  RCRET(rc);
  if (nlen_blk < olen_blk) {
    rc = _fsm_blk_deallocate_lw(impl, oaddr_blk + nlen_blk, olen_blk - nlen_blk);
    if (!rc) {
      *oaddr = oaddr_blk << impl->bpow;
      *olen = nlen_blk << impl->bpow;
    }
  } else {
    rc = _fsm_blk_allocate_lw(impl, nlen_blk, &naddr_blk, &sp, opts);
    RCGO(rc, finish);
    if (naddr_blk != oaddr_blk) {
      rc = impl->pool.copy(&impl->pool, *oaddr, (size_t) * olen, naddr_blk << impl->bpow);
      RCGO(rc, finish);
    }
    rc = _fsm_blk_deallocate_lw(impl, oaddr_blk, olen_blk);
    RCGO(rc, finish);
    *oaddr = naddr_blk << impl->bpow;
    *olen = sp << impl->bpow;
  }

finish:
  IWRC(_fsm_ctrl_unlock(impl), rc);
  return rc;
}

static iwrc _fsm_deallocate(struct IWFS_FSM *f, off_t addr, off_t len) {
  FSM_ENSURE_OPEN2(f);
  iwrc rc;
  FSM *impl = f->impl;
  off_t offset_blk = (uint64_t) addr >> impl->bpow;
  off_t length_blk = (uint64_t) len >> impl->bpow;

  if (!(impl->omode & IWFS_OWRITE)) {
    return IW_ERROR_READONLY;
  }
  if (addr & ((1ULL << impl->bpow) - 1)) {
    return IWFS_ERROR_RANGE_NOT_ALIGNED;
  }
  rc = _fsm_ctrl_wlock(impl);
  RCRET(rc);
  if (IW_RANGES_OVERLAP(offset_blk, offset_blk + length_blk, 0, (impl->hdrlen >> impl->bpow)) ||
      IW_RANGES_OVERLAP(offset_blk, offset_blk + length_blk, (impl->bmoff >> impl->bpow),
                        (impl->bmoff >> impl->bpow) + (impl->bmlen >> impl->bpow))) {
    // Deny deallocations in header or free-space bitmap itself
    IWRC(_fsm_ctrl_unlock(impl), rc);
    return IWFS_ERROR_FSM_SEGMENTATION;
  }
  rc = _fsm_blk_deallocate_lw(impl, (uint64_t) offset_blk, (uint64_t) length_blk);
  IWRC(_fsm_ctrl_unlock(impl), rc);
  return rc;
}

static iwrc _fsm_check_allocation_status(struct IWFS_FSM *f, off_t addr, off_t len, bool allocated) {
  FSM *impl = f->impl;
  if ((addr & ((1ULL << impl->bpow) - 1)) || (len & ((1ULL << impl->bpow) - 1))) {
    return IWFS_ERROR_RANGE_NOT_ALIGNED;
  }
  iwrc rc = _fsm_ctrl_rlock(impl);
  RCRET(rc);
  off_t offset_blk = (uint64_t) addr >> impl->bpow;
  off_t length_blk = (uint64_t) len >> impl->bpow;
  if (IW_RANGES_OVERLAP(offset_blk, offset_blk + length_blk, 0, (impl->hdrlen >> impl->bpow)) ||
      IW_RANGES_OVERLAP(offset_blk, offset_blk + length_blk, (impl->bmoff >> impl->bpow),
                        (impl->bmoff >> impl->bpow) + (impl->bmlen >> impl->bpow))) {
    IWRC(_fsm_ctrl_unlock(impl), rc);
    return IWFS_ERROR_FSM_SEGMENTATION;
  }
  rc = _fsm_set_bit_status_lw(impl, (uint64_t) offset_blk, (uint64_t) length_blk,
                              allocated ? 0 : 1, FSM_BM_DRY_RUN | FSM_BM_STRICT);
  IWRC(_fsm_ctrl_unlock(impl), rc);
  return rc;
}

static iwrc _fsm_writehdr(struct IWFS_FSM *f, off_t off, const void *buf, off_t siz) {
  FSM_ENSURE_OPEN2(f);
  iwrc rc;
  uint8_t *mm;
  if (siz < 1) return 0;
  FSM *impl = f->impl;
  if ((IWFSM_CUSTOM_HDR_DATA_OFFSET + off + siz) > impl->hdrlen) {
    return IW_ERROR_OUT_OF_BOUNDS;
  }
  rc = impl->pool.acquire_mmap(&impl->pool, 0, &mm, 0);
  if (!rc) {
    if (impl->dlsnr) {
      rc = impl->dlsnr->onwrite(impl->dlsnr, IWFSM_CUSTOM_HDR_DATA_OFFSET + off, buf, siz, 0);
    }
    memmove(mm + IWFSM_CUSTOM_HDR_DATA_OFFSET + off, buf, (size_t) siz);
    IWRC(impl->pool.release_mmap(&impl->pool), rc);
  }
  return rc;
}

static iwrc _fsm_readhdr(struct IWFS_FSM *f, off_t off, void *buf, off_t siz) {
  FSM_ENSURE_OPEN2(f);
  iwrc rc;
  uint8_t *mm;
  if (siz < 1) return 0;
  FSM *impl = f->impl;
  if ((IWFSM_CUSTOM_HDR_DATA_OFFSET + off + siz) > impl->hdrlen) {
    return IW_ERROR_OUT_OF_BOUNDS;
  }
  rc = impl->pool.acquire_mmap(&impl->pool, 0, &mm, 0);
  if (!rc) {
    memmove(buf, mm + IWFSM_CUSTOM_HDR_DATA_OFFSET + off, (size_t) siz);
    rc = impl->pool.release_mmap(&impl->pool);
  }
  return rc;
}

static iwrc _fsm_clear(struct IWFS_FSM *f, iwfs_fsm_clrfalgs clrflags) {
  FSM_ENSURE_OPEN2(f);
  FSM *impl = f->impl;
  uint64_t bmoff, bmlen;
  iwrc rc = _fsm_ctrl_wlock(impl);
  bmlen = impl->bmlen;
  if (!bmlen) {
    goto finish;
  }
  if (!impl->mmap_all && impl->bmoff) {
    IWRC(impl->pool.remove_mmap(&impl->pool, impl->bmoff), rc);
  }
  bmoff = IW_ROUNDUP(impl->hdrlen, impl->aunit);
  if (!impl->mmap_all) {
    IWRC(impl->pool.add_mmap(&impl->pool, bmoff, bmlen, impl->mmap_opts), rc);
  }
  RCGO(rc, finish);
  impl->bmlen = 0;
  impl->bmoff = 0;
  rc = _fsm_init_lw(impl, bmoff, bmlen);
  if (!rc && (clrflags & IWFSM_CLEAR_TRIM)) {
    rc = _fsm_trim_tail_lw(impl);
  }

finish:
  IWRC(_fsm_ctrl_unlock(impl), rc);
  return rc;
}

static iwrc _fsm_extfile(struct IWFS_FSM *f, IWFS_EXT **ext) {
  FSM_ENSURE_OPEN2(f);
  *ext = &f->impl->pool;
  return 0;
}

static iwrc _fsm_state(struct IWFS_FSM *f, IWFS_FSM_STATE *state) {
  FSM_ENSURE_OPEN2(f);
  FSM *impl = f->impl;
  iwrc rc = _fsm_ctrl_rlock(impl);
  memset(state, 0, sizeof(*state));
  IWRC(impl->pool.state(&impl->pool, &state->exfile), rc);
  state->block_size = 1U << impl->bpow;
  state->oflags = impl->oflags;
  state->hdrlen = impl->hdrlen;
  state->blocks_num = impl->bmlen << 3;
  state->free_segments_num = (uint64_t) kb_size(impl->fsm);
  state->avg_alloc_size = impl->crznum > 0 ? (double_t) impl->crzsum / (double_t) impl->crznum : 0;
  state->alloc_dispersion = impl->crznum > 0 ? (double_t) impl->crzvar / (double_t) impl->crznum : 0;
  IWRC(_fsm_ctrl_unlock(impl), rc);
  return rc;
}

iwrc iwfs_fsmfile_open(IWFS_FSM *f, const IWFS_FSM_OPTS *opts) {
  assert(f && opts);
  iwrc rc = 0;
  IWFS_EXT_STATE fstate = {0};
  const char *path = opts->exfile.file.path;

  memset(f, 0, sizeof(*f));
  rc = iwfs_fsmfile_init();
  RCGO(rc, finish);

  f->write = _fsm_write;
  f->read = _fsm_read;
  f->close = _fsm_close;
  f->sync = _fsm_sync;
  f->state = _fsm_state;

  f->ensure_size = _fsm_ensure_size;
  f->add_mmap = _fsm_add_mmap;
  f->remap_all = _fsm_remap_all;
  f->acquire_mmap = _fsm_acquire_mmap;
  f->probe_mmap = _fsm_probe_mmap;
  f->release_mmap = _fsm_release_mmap;
  f->remove_mmap = _fsm_remove_mmap;
  f->sync_mmap = _fsm_sync_mmap;

  f->allocate = _fsm_allocate;
  f->reallocate = _fsm_reallocate;
  f->deallocate = _fsm_deallocate;
  f->check_allocation_status = _fsm_check_allocation_status;
  f->writehdr = _fsm_writehdr;
  f->readhdr = _fsm_readhdr;
  f->clear = _fsm_clear;
  f->extfile = _fsm_extfile;

  if (!path) {
    return IW_ERROR_INVALID_ARGS;
  }
  FSM *impl = f->impl = calloc(1, sizeof(*f->impl));
  if (!impl) {
    return iwrc_set_errno(IW_ERROR_ALLOC, errno);
  }
  impl->f = f;
  impl->dlsnr = opts->exfile.file.dlsnr; // Copy data changes listener address
  impl->mmap_opts = opts->mmap_opts;

  IWFS_EXT_OPTS rwl_opts = opts->exfile;
  rwl_opts.use_locks = !(opts->oflags & IWFSM_NOLOCKS);

  rc = _fsm_init_impl(impl, opts);
  RCGO(rc, finish);

  rc = _fsm_init_locks(impl, opts);
  RCGO(rc, finish);

  rc = iwfs_exfile_open(&impl->pool, &rwl_opts);
  RCGO(rc, finish);

  rc = impl->pool.state(&impl->pool, &fstate);
  RCGO(rc, finish);

  impl->omode = fstate.file.opts.omode;

  if (fstate.file.ostatus & IWFS_OPEN_NEW) {
    rc = _fsm_init_new_lw(impl, opts);
  } else {
    rc = _fsm_init_existing_lw(impl);
  }

finish:
  if (rc) {
    if (f->impl) {
      IWRC(_fsm_destroy_locks(f->impl), rc);  // we are not locked
      IWRC(_fsm_close(f), rc);
    }
  }
  return rc;
}

static const char *_fsmfile_ecodefn(locale_t locale, uint32_t ecode) {
  if (!(ecode > _IWFS_FSM_ERROR_START && ecode < _IWFS_FSM_ERROR_END)) {
    return 0;
  }
  switch (ecode) {
    case IWFS_ERROR_NO_FREE_SPACE:
      return "No free space. (IWFS_ERROR_NO_FREE_SPACE)";
    case IWFS_ERROR_INVALID_BLOCK_SIZE:
      return "Invalid block size specified. (IWFS_ERROR_INVALID_BLOCK_SIZE)";
    case IWFS_ERROR_RANGE_NOT_ALIGNED:
      return "Specified range/offset is not aligned with page/block. "
             "(IWFS_ERROR_RANGE_NOT_ALIGNED)";
    case IWFS_ERROR_FSM_SEGMENTATION:
      return "Free-space map segmentation error. (IWFS_ERROR_FSM_SEGMENTATION)";
    case IWFS_ERROR_INVALID_FILEMETA:
      return "Invalid file metadata. (IWFS_ERROR_INVALID_FILEMETA)";
    case IWFS_ERROR_PLATFORM_PAGE:
      return "The block size incompatible with platform page size, data "
             "migration required. (IWFS_ERROR_PLATFORM_PAGE)";
    case IWFS_ERROR_RESIZE_FAIL:
      return "Failed to resize file, "
             "conflicting with free-space map location (IWFS_ERROR_RESIZE_FAIL)";
    default:
      break;
  }
  return 0;
}

iwrc iwfs_fsmfile_init(void) {
  static int _fsmfile_initialized = 0;
  iwrc rc = iw_init();
  RCRET(rc);
  if (!__sync_bool_compare_and_swap(&_fsmfile_initialized, 0, 1)) {
    return 0;  // initialized already
  }
  return iwlog_register_ecodefn(_fsmfile_ecodefn);
}

/*************************************************************************************************
 *                                      Debug API                                                *
 *************************************************************************************************/

uint64_t iwfs_fsmdbg_number_of_free_areas(IWFS_FSM *f) {
  int ret;
  assert(f);
  FSM *impl = f->impl;
  _fsm_ctrl_rlock(impl);
  ret = kb_size(impl->fsm);
  _fsm_ctrl_unlock(impl);
  return (uint64_t) ret;
}

uint64_t iwfs_fsmdbg_find_next_set_bit(const uint64_t *addr, uint64_t offset_bit, uint64_t max_offset_bit,
                                       int *found) {
  return _fsm_find_next_set_bit(addr, offset_bit, max_offset_bit, found);
}

uint64_t iwfs_fsmdbg_find_prev_set_bit(const uint64_t *addr, uint64_t offset_bit, uint64_t min_offset_bit,
                                       int *found) {
  return _fsm_find_prev_set_bit(addr, offset_bit, min_offset_bit, found);
}

void iwfs_fsmdbg_dump_fsm_tree(IWFS_FSM *f, const char *hdr) {
  assert(f);
  FSM *impl = f->impl;
  fprintf(stderr, "FSM TREE: %s\n", hdr);
  if (!impl->fsm) {
    fprintf(stderr, "NONE\n");
    return;
  }
#define _fsm_traverse(k)                                                                                    \
  {                                                                                                         \
    uint64_t koff = FSMBK_OFFSET(k);                                                                        \
    uint64_t klen = FSMBK_LENGTH(k);                                                                        \
    fprintf(stderr, "[%" PRIu64 " %" PRIu64 "]\n", koff, klen);                                             \
  }
  __kb_traverse(FSMBK, impl->fsm, _fsm_traverse);
#undef _fsm_traverse
}


const char *byte_to_binary(int x) {
  static char b[9];
  b[0] = '\0';
  int z;
  for (z = 1; z <= 128; z <<= 1) {
    strcat(b, ((x & z) == z) ? "1" : "0");
  }
  return b;
}


iwrc iwfs_fsmdb_dump_fsm_bitmap(IWFS_FSM *f) {
  assert(f);
  size_t sp;
  uint8_t *mm;
  FSM *impl = f->impl;
  iwrc rc;
  if (impl->mmap_all) {
    rc = impl->pool.probe_mmap(&impl->pool, 0, &mm, &sp);
    if (!rc) {
      if (sp <= impl->bmoff) {
        rc = IWFS_ERROR_NOT_MMAPED;
      } else {
        mm += impl->bmoff;
        sp = sp - impl->bmoff;
      }
    }
  } else {
    rc = impl->pool.probe_mmap(&impl->pool, impl->bmoff, &mm, &sp);
  }
  if (rc) {
    iwlog_ecode_error3(rc);
    return rc;
  }
  int i = ((impl->hdrlen >> impl->bpow) >> 3);
  // if (impl->bmoff == impl->aunit) {
  //   i += ((impl->bmlen >> impl->bpow) >> 3);
  // }
  for (; i < sp && i < impl->bmlen; ++i) {
    uint8_t b = *(mm + i);
    fprintf(stderr, "%s", byte_to_binary(b));
  }
  printf("\n");
  return 0;
}


iwrc iwfs_fsmdbg_state(IWFS_FSM *f, IWFS_FSMDBG_STATE *d) {
  FSM_ENSURE_OPEN2(f);
  FSM *impl = f->impl;
  iwrc rc = _fsm_ctrl_rlock(impl);
  memset(d, 0, sizeof(*d));
  IWRC(impl->pool.state(&impl->pool, &d->state.exfile), rc);
  d->state.block_size = 1U << impl->bpow;
  d->state.oflags = impl->oflags;
  d->state.hdrlen = impl->hdrlen;
  d->state.blocks_num = impl->bmlen << 3;
  d->state.free_segments_num = (uint64_t) kb_size(impl->fsm);
  d->state.avg_alloc_size = impl->crznum > 0 ? (double_t) impl->crzsum / (double_t) impl->crznum : 0;
  d->state.alloc_dispersion = impl->crznum > 0 ? (double_t) impl->crzvar / (double_t) impl->crznum : 0;
  d->bmoff = impl->bmoff;
  d->bmlen = impl->bmlen;
  d->lfbkoff = impl->lfbkoff;
  d->lfbklen = impl->lfbklen;
  IWRC(_fsm_ctrl_unlock(impl), rc);
  return rc;
}
