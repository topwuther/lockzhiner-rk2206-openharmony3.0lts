import click
import sys, os, struct, datetime, io, re
import json as js
from Crypto.Hash import SHA256
from recordclass import recordclass
RESC_HEAD_FORMAT = ('<8s'   # magic
                    '16s'   # chip
                    '32s'   # model
                    '16s'   # description
                    '4s'    # version
                    '4s'    # release date
                    'I'     # data offset
                    'I'     # data size
                    '4s'    # reserved
                    'I'     # hash 1:jshash; 2:sha256
                    'I'     # rsa flag
                    '32s'   # custom data
                    '380s') # reserved
VERSION_FORMAT = ('<B' #major
                  'B'  #minor
                  'H') #small
DATE_FORMAT = ('<H' #year
                'B'  #month
                'B') #day

VERSION = "1.3"
# Short description
DESCRIPTION = (
    'toolkit for resource header'
)
PAGE_SIZE = 512
SECTOR_SIZE = 512
def align(size, unit):
    size = size + unit - 1
    size = size & (~ (unit - 1))
    return size
def string_to_fix_bytes(str, fix_size):
    ret_bytes = bytearray(fix_size)
    str_size = len(str)
    if fix_size >= str_size:
        ret_bytes[0:str_size] = str.encode()
    else:
        ret_bytes[:] = str[0:fix_size].encode()
    return ret_bytes
def save_file(fout, fin, in_file_size):
    write_byte = 0
    while in_file_size > 0:
        if in_file_size < PAGE_SIZE:
            write_byte = in_file_size
        else:
            write_byte = PAGE_SIZE
        buffer = fin.read(write_byte)
        fout.write(buffer)
        in_file_size -= write_byte
def js_hash(init, buf, size):
    i = 0
    mask = 2 ** 32 - 1
    hash = init & mask
    while i < size:
        left = (hash << 5) & mask
        right = (hash >> 2) & mask
        mid = buf[i] & mask
        hash = hash ^ (left + mid + right)
        hash = hash & mask
        i = i + 1
    return hash
def do_file_js_hash(fin, hash_size):
    fin.seek(0, io.SEEK_SET)
    hash = 0
    while hash_size > 0:
        if hash_size >= PAGE_SIZE:
            buf = fin.read(PAGE_SIZE)
        else:
            buf = fin.read(hash_size)
        hash = js_hash(hash, buf, len(buf))
        hash_size = hash_size - len(buf)
    return hash
def do_sha256(message, digest, init=None):
    try:
        sha_obj = init
        if sha_obj is None:
            sha_obj = SHA256.new()
        blk_size = sha_obj.block_size
        msg_size = len(message)
        update_size = 0
        pos = 0
        while msg_size > 0 :
            if msg_size >= blk_size:
                update_size = blk_size
            else:
                update_size = msg_size
            sha_obj.update(message[pos:pos+update_size])
            pos += update_size
            msg_size -= update_size

        digest[:] = sha_obj.digest()
        return True
    except Exception as e:
        click.secho(str(e), fg='red')
    return False
def do_sha256_file(fp,file_size,digest):
    try:
        sha_obj = SHA256.new()
        blk_size = sha_obj.block_size
        msg_size = file_size
        update_size = 0
        fp.seek(0, io.SEEK_SET)
        while msg_size > 0:
            if msg_size >= blk_size:
                update_size = blk_size
            else:
                update_size = msg_size
            sha_obj.update(fp.read(update_size))
            msg_size -= update_size

        digest[:] = sha_obj.digest()
        return True
    except Exception as e:
        click.secho(str(e),fg='red')
    return False
def get_current_dir():
    if sys.platform.startswith('win'):
        exe_file = sys.executable.split('\\')[-1]
    else:
        exe_file = sys.executable.split('/')[-1]
    if exe_file.find('python') == -1:
        dir = os.path.dirname(sys.executable)
    else:
        dir = os.path.dirname(os.path.abspath(__file__))
    return dir
# Base options
@click.group(context_settings=dict(help_option_names=['-h', '--help']), help=DESCRIPTION)
@click.version_option(VERSION, '-v', '--version')
def cli():
    pass
@cli.command(name='pack', short_help='generate head and hash for any file')
@click.argument('file', nargs=1, type=click.Path(exists=True))
@click.option('--json', type=click.Path(exists=True), help='config file using json format')
def pack_header(file, json):
    cwd = get_current_dir()
    if json is None:
        json = os.path.join(cwd, 'config.json')
        if not os.path.exists(json):
            click.secho('%s is not existed' % (json), fg='red')
            return
    click.echo('file=%s,json=%s' % (file,json))
    head_size = struct.calcsize(RESC_HEAD_FORMAT)
    head_buf = bytearray(head_size)
    head_struct = list(struct.unpack(RESC_HEAD_FORMAT, head_buf))
    ver_struct = list(struct.unpack(VERSION_FORMAT, head_struct[4]))
    date_struct = list(struct.unpack(DATE_FORMAT, head_struct[5]))
    HEAD_RECORD = recordclass('H', 'magic chip model desc ver date offset size reserved hash_flag sign_flag data reserved2')
    VERSION_RECORD = recordclass('V', 'major minor small')
    DATE_RECORD = recordclass('D', 'year month day')
    release_ver = VERSION_RECORD(*ver_struct)
    release_date = DATE_RECORD(*date_struct)
    resc_head = HEAD_RECORD(*head_struct)

    with open(json, 'r') as f:
        configs = js.loads(f.read())
    if not configs:
        click.secho('failed to load %s' % (json), fg='red')
        return
    with open(file, 'rb') as fin:
        if os.path.getsize(file) > PAGE_SIZE:
            magic = fin.read(8)
            if magic.decode(errors='ignore').startswith('RESC') or magic.decode(errors='ignore').startswith('RESS'):
                click.secho('%s has repacked' % (file), fg='red')
                return

    src_size = len(resc_head.magic)
    resc_head.magic = string_to_fix_bytes('RESC', src_size)
    if 'CHIP' not in configs:
        click.secho('No found key of CHIP', fg='red')
        return
    src_size = len(resc_head.chip)
    str_size = len(configs['CHIP'])
    if str_size > 0:
        resc_head.chip = string_to_fix_bytes(configs['CHIP'], src_size)

    if 'MODEL' not in configs:
        click.secho('No found key of MODEL', fg='red')
        return
    src_size = len(resc_head.model)
    str_size = len(configs['MODEL'])
    if str_size > 0:
        resc_head.model = string_to_fix_bytes(configs['MODEL'], src_size)

    if 'DESC' not in configs:
        click.secho('No found key of DESC', fg='red')
        return
    src_size = len(resc_head.desc)
    str_size = len(configs['DESC'])
    if str_size > 0:
        resc_head.desc = string_to_fix_bytes(configs['DESC'], src_size)

    if 'VERSION' not in configs:
        click.secho('No found key of VERSION', fg='red')
        return
    ver_list = configs['VERSION'].split('.')
    if len(ver_list) > 0:
        if ver_list[0].isdigit():
            release_ver.major = int(ver_list[0])
        else:
            click.secho('VERSION is invalid', fg='red')
            return
        if len(ver_list) > 1:
            if ver_list[1].isdigit():
                release_ver.minor = int(ver_list[1])
            else:
                click.secho('VERSION is invalid', fg='red')
                return
        if len(ver_list) > 2:
            if ver_list[2].isdigit():
                release_ver.small = int(ver_list[2])
            else:
                click.secho('VERSION is invalid', fg='red')
                return
        resc_head.ver = struct.pack(VERSION_FORMAT, *release_ver)
    now = datetime.datetime.now()
    release_date.year = now.year
    release_date.month = now.month
    release_date.day = now.day
    resc_head.date = struct.pack(DATE_FORMAT, *release_date)
    resc_head.offset = PAGE_SIZE
    file_size = os.path.getsize(file)
    resc_head.size = align(file_size,SECTOR_SIZE)
    if 'DIGEST' not in configs:
        click.secho('No found key of DIGEST', fg='red')
        return
    if len(configs['DIGEST']) == 0:
        click.secho('DIGEST is empty', fg='red')
        return
    if bool(re.fullmatch(configs['DIGEST'], 'JSHASH', re.IGNORECASE)):
        resc_head.hash_flag = 1
    elif bool(re.fullmatch(configs['DIGEST'], 'SHA256', re.IGNORECASE)):
        resc_head.hash_flag = 2
    else:
        click.secho('DIGEST of %s still not support' % (configs['DIGEST']), fg='red')
        return
    if 'DATA' in configs:
        custom_file = configs['DATA']
        if os.path.exists(custom_file):
            resc_head.data = bytearray(resc_head.data)
            with open(custom_file,'rb') as fp:
                custom_data = fp.read()
                if len(custom_data)>32:
                    resc_head.data = custom_data[0:32]
                else:
                    resc_head.data[0:len(custom_data)] = custom_data
    head_buf[:] = struct.pack(RESC_HEAD_FORMAT, *resc_head)

    tmp_file = file + '.tmp'
    with open(tmp_file, 'wb+') as fout, open(file, 'rb') as fin:
        fout.write(head_buf)
        fout.write(bytearray(PAGE_SIZE-head_size))
        save_file(fout, fin, file_size)
        if resc_head.size != file_size:
            fout.write(bytearray(resc_head.size - file_size))
        fout.flush()
        if resc_head.hash_flag == 1:
            hash = do_file_js_hash(fout, resc_head.offset + resc_head.size)
            fout.write(hash.to_bytes(4, byteorder='little'))
        elif resc_head.hash_flag == 2:
            hash = bytearray(32)
            if do_sha256_file(fout,resc_head.offset + resc_head.size,hash):
                fout.write(hash)
            else:
                click.secho('Failed to do sha256', fg='red')
                return

    os.remove(file)
    os.rename(tmp_file,file)
    click.echo('Packing %s done' % (os.path.basename(file)))


@cli.command(name='unpack', short_help='extract source from packing file')
@click.argument('source', nargs=1, type=click.Path(exists=True))
@click.argument('out', nargs=1, type=click.Path())
def unpack_header(source, out):
    click.echo('source=%s,out=%s' % (source, out))
    head_size = struct.calcsize(RESC_HEAD_FORMAT)
    with open(source, 'rb') as fin:
        head_buf = fin.read(head_size)
        head_struct = list(struct.unpack(RESC_HEAD_FORMAT, head_buf))
        HEAD_RECORD = recordclass('H', 'magic chip model desc ver date offset size reserved hash_flag sign_flag hash reserved2')
        resc_head = HEAD_RECORD(*head_struct)
        if (not resc_head.magic.decode(errors='ignore').startswith('RESC')) and (not resc_head.magic.decode(errors='ignore').startswith('RESS')):
            click.secho('failed to check magic,%s is invalid' % resc_head.magic.decode(errors='ignore'), fg='red')
            return
        if resc_head.hash_flag == 1 :
            if (resc_head.offset + resc_head.size + 4) != os.path.getsize(source):
                click.secho('failed to check size', fg='red')
                return
            hash = do_file_js_hash(fin, resc_head.offset + resc_head.size)
            fin.seek(-4, io.SEEK_END)
            hash_buf = fin.read(4)
            old_hash = int.from_bytes(hash_buf, byteorder='little')
            if hash != old_hash:
                click.secho('failed to check hash,0x%x!=0x%x' % (hash, old_hash), fg='red')
                return
        elif resc_head.hash_flag == 2:
            if (resc_head.offset + resc_head.size + 32) != os.path.getsize(source):
                click.secho('failed to check size', fg='red')
                return
            hash = bytearray(32)
            do_sha256_file(fin,resc_head.offset+resc_head.size,hash)
            fin.seek(-32, io.SEEK_END)
            old_hash = fin.read(32)
            if hash != old_hash:
                click.secho('failed to check hash,{}!={}'.format(repr(hash), repr(old_hash)), fg='red')
                return
        else:
            click.secho('invalid hash',fg='red')
        fin.seek(resc_head.offset, io.SEEK_SET)
        with open(out,'wb') as fout:
            save_file(fout,fin,resc_head.size)
    click.echo('Unpacking %s done' % out)


if __name__ == '__main__':
    cli()
