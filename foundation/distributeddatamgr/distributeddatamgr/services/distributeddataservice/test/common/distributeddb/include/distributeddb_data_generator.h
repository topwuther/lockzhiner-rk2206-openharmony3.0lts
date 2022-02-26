/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef DISTRIBUTED_DB_MODULE_TEST_TYPES_H
#define DISTRIBUTED_DB_MODULE_TEST_TYPES_H

#include <string>
#include <vector>
#include <cmath>
#include "types.h"
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "distributed_test_sysinfo.h"
#include "distributeddb_log_print.h"

struct EntrySize {
    unsigned int keySize = 0;
    unsigned int valSize = 0;
    void Set(unsigned int keySizeInput, unsigned int valSizeInput)
    {
        this->keySize = keySizeInput;
        this->valSize = valSizeInput;
    }
};

enum DBType {
    UNENCRYPTED_DISK_DB = 0,
    ENCRYPTED_DISK_DB,
    MEMORY_DB
};

struct TimeSamp {
    std::vector<uint64_t> startTime;
    std::vector<uint64_t> endTime;
    TimeSamp()
    {
    }
};

struct ImagePerSamp {
    std::vector<uint64_t> firstStep;
    std::vector<uint64_t> secondStep;
    ImagePerSamp()
    {
    }
};

struct PerTimeLength {
    std::vector<double> entriesDuration;
    std::vector<double> cursorDuration;
    std::vector<double> moveNextDuration;
    std::vector<double> entryDurationForward;
    std::vector<double> movePreviousDuration;
    std::vector<double> entryDurationBackup;
};

struct Schema {
    std::vector<std::string> version;
    std::vector<std::string> mode;
    std::vector<std::string> define;
    std::vector<std::string> index;
};
struct LongDefine {
    int recordNum;
    int recordSize;
    char prefix;
};

enum class RandType {
    ALPHA_NUM,
    ALPHA_NUM_UNDERLINE,
    SPECIAL,
};

// ************************  number class  **********************************
const unsigned int OBSERVER_NUM = 8;
const int BUFFER_COUNT = 5;
const unsigned int NB_OBSERVER_NUM = 4;
const unsigned int NB_OPERATION_NUM = 100;
const unsigned int NB_PREDATA_NUM = 50;
const unsigned int RECORDS_SMALL_CNT = 2;
const unsigned int NO_RECORD = 0;
const unsigned int THREE_RECORDS = 3;
const unsigned int THREE_PERF_DATA = 3;
const unsigned int BATCH_RECORDS = 128;
const static int ONE_RECORD = 1;
const static int FOUR_RECORDS = 4;
const static int FIVE_RECORDS = 5;
const static int TEN_RECORDS = 10;
const static int TWENTY_RECORDS = 20;
const static int THIRTYTWO_RECORDS = 32;
const static int FORTY_RECORDS = 40;
const static int FIFTY_RECORDS = 50;
const static int SIXTY_RECORDS = 60;
const static int EIGHTY_RECORDS = 80;
const static int ONE_HUNDRED_RECORDS = 100;
const static int ONE_HUNDRED_AND_TWENTY_RECORDS = 120;
const static int TWO_HUNDREDS_RECORDS = 200;
const static int FIVE_HUNDREDS_RECORDS = 500;
const static int SIX_HUNDREDS_RECORDS = 600;
const static int ONE_THOUSAND_RECORDS = 1000;
const static int TWO_THOUSANDS_RECORDS = 2000;
const static int TWO_FIVE_ZERO_ZERO_RECORDS = 2500;
const static int TWO_FIVE_SIX_ZERO_RECORDS = 2560;
const static int FIVE_THOUSANDS_RECORDS = 5000;
const static int TEN_THOUSAND_RECORDS = 10000;
const static int TWENTY_THOUSAND_RECORDS = 20000;
const static int HUNDRED_THOUSAND_RECORDS = 100000;
const static int FOUR_HUNDRED_THOUSAND_RECORDS = 400000;
const static int FIVE_HUNDRED_THOUSAND_RECORDS = 500000;
const static int TWO_FIVE_SIX_RECORDS = 256;
const static int FIRST_RECORD = 1;
const static int SECOND_RECORD = 2;
const static int FOURTH_RECORD = 4;
const static int FORTIETH_RECORD = 40;
const static int EIGHTIETH_RECORD = 80;
const static int THE_HUNDRED_AND_TWENTY_RECORD = 120;
const static int DATAS_ACCOUNT = 300;
const static int DATA_LEN = 13;
const static int NINE_CNT = 9;
const static unsigned int TWO_DEVICES = 2;
const static unsigned int FOUR_DEVICES = 4;
const static int THIRD_FUNC = 3;
const static int FOURTH_FUNC = 4;
const static int FIFTH_FUNC = 5;
const static int THREE_DBS = 3;
const static int EIGHT_DBS = 8;
const static int TEN_DBS = 10;
const static int ELEVEN_DBS = 11;
const static int FIRST_DB = 1;
const static int FIFTH_DB = 5;
const static int TENTH_DB = 10;
const static int ELEVENTH_DB = 11;

// ************************  loop times class      **************************
const unsigned int MOD_NUM = 2;
const unsigned int BIG_MOD_NUM = 3;

const unsigned int ID_CNT_START = 0;
const unsigned int ID_MIN_CNT = 2;
const unsigned int ID_MEDIUM_CNT = 3;
const unsigned int ID_MAX_CNT = 16;
const unsigned int STORE_CNT = 4;
const unsigned int DIR_CNT_START = 0;
const unsigned int DIR_MAX_CNT = 14;
const static int MANYTINES = 3;
const static int ONE_TIME = 1;
const static int TWO_TIMES = 2;
const static int FIVE_TIMES = 5;
const static int FIFTY_TIMES = 50;
const static int HUNDRED_TIMES = 100;

// ************************  time class            **************************
const unsigned int UNIQUE_SECOND = 1;
const unsigned int TWO_SECONDS = 2;
const unsigned int THREE_SECONDS = 3;
const unsigned int FOUR_SECONDS = 4;
const unsigned int FIVE_SECONDS = 5;
const unsigned int TEN_SECONDS = 10;
const unsigned int FIFTEEN_SECONDS = 15;
const unsigned int TWENTY_SECONDS = 20;
const unsigned int THIRTY_SECONDS = 30;
const unsigned int FORTY_SECONDS = 40;
const unsigned int SIXTY_SECONDS = 60;
const unsigned int ONE_HUNDRED_SECONDS = 100;
const unsigned int TWO_HUNDREDS_SECONDS = 200;
const unsigned int THREE_HUNDREDS_SECONDS = 300;
const unsigned int FOUR_HUNDREDS_SECONDS = 400;
const unsigned int FIVE_HUNDREDS_SECONDS = 500;
const unsigned int SIX_HUNDREDS_SECONDS = 600;
const unsigned int SEVEN_HUNDREDS_SECONDS = 700;
const unsigned int EIGHT_HUNDREDS_SECONDS = 800;
const unsigned int TEN_HUNDREDS_SECONDS = 1000;
const unsigned int ONE_FIVE_ZERO_ZERO_SECONDS = 1500;
const unsigned int SIXTEEN_HUNDREDS_SECONDS = 1600;
const unsigned int EIGHTEEN_HUNDREDS_SECONDS = 1800;
const unsigned int TWENTY_HUNDREDS_SECONDS = 2000;
const unsigned int THREE_THOUSANDS_SECONDS = 3000;
const unsigned int THREE_SIX_ZERO_ZERO_SECONDS = 3600;
const unsigned int EIGHT_THOUSANDS_SECONDS = 8000;
const unsigned int TEN_THOUSANDS_SECONDS = 10000;
const unsigned int WAIT_FOR_LONG_TIME = 15000;
const unsigned int WAIT_FOR_FIFTY_MS = 50000;
const unsigned int WAIT_FOR_LAST_SYNC = 500000;
const unsigned int WAIT_FOR_TWO_HUNDREDS_MS = 200000;
const int FIFTY_MILI_SECONDS = 50;
const int HUNDRED_MILLI_SECONDS = 100;
const static int MILLSECONDES_PER_SECOND = 1000;

// ************************  length of key class   **************************
const static int KEY_SIX_BYTE = 6;
const static int KEY_EIGHT_BYTE = 8;
const static int KEY_THIRTYTWO_BYTE = 32;
const static int KEY_SIXTYFOUR_BYTE = 64;
const static int KEY_TWO_FIVE_SIX_BYTE = 256;
const static int KEY_ONE_K_BYTE = 1024;
const static int KEY_ONE_HUNDRED_BYTE = 100;

// ************************  length of value class **************************
const unsigned int SMALL_VALUE_SIZE = 2;
const unsigned int ONE_K_LONG_STRING = 1024; // 1K
const unsigned int TWO_K_LONG_STRING = 2048; // 2K
const unsigned int TWO_POINT_FOUR_LONG = 2400; // 2.4K
const unsigned int THREE_K_LONG_STRING = 3072; // 3K
const unsigned int FOUR_K_LONG_STRING = 4096; // 4K
const unsigned int ONE_M_LONG_STRING = 1048576; // 1M
const unsigned int TWO_M_LONG_STRING = 2097152; // 2M
const unsigned int FOUR_M_LONG_STRING = 4194304; // 4M
const unsigned int TEN_M_LONG_STRING = 10485760; // 10M
const static int VALUE_ONE_HUNDRED_BYTE = 100;
const static int VALUE_FIVE_HUNDRED_BYTE = 500;
const static int VALUE_ONE_K_BYTE = 1024;
const static int VALUE_HUNDRED_K_BYTE = 102400;
const static int VALUE_TWENTY_K_BYTE = 20480;
const static int VALUE_TWO_POINT_FOUR_K_BYTE = 2458;
const static int PASSWD_BYTE = 129;

// ************************  bool class *************************************
const bool IS_LOCAL_ONLY = true;
const bool IS_NOT_LOCAL_ONLY = false;
const bool IS_NEED_CREATE = true;
const bool IS_NOT_NEED_CREATE = false;

// ************************  range verify class *****************************
const static int CHANGED_ZERO_TIME = 0;
const static int CHANGED_ONE_TIME = 1;
const static int CHANGED_TWO_TIMES = 2;
const static int CHANGED_THREE_TIMES = 3;
const static int CHANGED_ONE_HUNDRED_TIMES = 100;
const static int CHANGED_TWO_HUNDRED_TIMES = 200;
const static int CHANGED_TWO_FIVE_SIX_TIMES = 256;
const static int CHANGED_ONE_THOUSAND_TIMES = 1000;
const static int CHANGED_TEN_THOUSAND_TIMES = 10000;
const unsigned int RECORDS_NUM_START = 1;
const unsigned int RECORDS_NUM_END = 128;
const unsigned int DEFAULT_START = 1;
const unsigned int DEFAULT_ANOTHER_START = 11;
const unsigned int OBSERVER_CNT_START = 0;
const unsigned int OBSERVER_CNT_END = 8;
const unsigned int NB_OBSERVER_CNT_START = 0;
const unsigned int NB_OBSERVER_CNT_END = 4;
const unsigned int NB_OPERATION_CNT_START = 0;
const unsigned int NB_OPERATION_CNT_END = 5;
const int RAND_BOOL_MIN = 0;
const int RAND_BOOL_MAX = 1;
const int LOCAL_OPER_CNT = 2;
const int NATIVE_OPER_CNT = 6;
const static int OPER_CNT_START = 0;
const static int OPER_CNT_END = 10;
const static int MODE_RAND_MIN = 0;
const static int MODE_RAND_MAX = 2;

// ************************  USED FOR THREAD ********************************
const unsigned int SINGLE_THREAD_NUM = 2;
const unsigned int CHAR_SPAN_MIN = 0;
const unsigned int CHAR_SPAN_MAX = 255;

// ************************  USED FOR INDEX *********************************
const int INDEX_ZEROTH = 0;
const int INDEX_FIRST = 1;
const int INDEX_SECOND = 2;
const int INDEX_THIRD = 3;
const int INDEX_FORTH = 4;
const int INDEX_FIFTH = 5;
const int INDEX_SIXTH = 6;
const int INDEX_SEVENTH = 7;
const int INDEX_EIGHTTH = 8;
const int INDEX_NINTH = 9;
const int INDEX_NINE_NINE_NINTH = 999;

// ************************  OTHER CLASS ************************************
const static int ROUND_BACK = 2;
const static int TRUNC_EIGHT = 100000000;

const unsigned int PIPE_BUFFER = 128;

const uint8_t ACSIIEND = 255;

const int TABLE_MAX = 256;

const static int ENCRYPT_COUNT = 100;

const static int EVEN_NUMBER = 2;

const int VALUE_FIVE_HUNDRED = 500;
const int VALUE_SUM = VALUE_FIVE_HUNDRED + VALUE_FIVE_HUNDRED;
const int VALUE_CHANGE1_FIRST = 400;
const int VALUE_CHANGE1_SECOND = VALUE_SUM - VALUE_CHANGE1_FIRST;
const int VALUE_CHANGE2_FIRST = 700;
const int VALUE_CHANGE2_SECOND = VALUE_SUM - VALUE_CHANGE2_FIRST;

const static std::string MULTIDB = "/multi_ver/value_storage.db";
const static std::string KVMULTIDB = "/multi_ver/multi_ver_data.db";
const static std::string SYNC_MULTI_VER_QUERY_SQL = "select count(*) from version_data;";
const static std::string DATABASE_INFOR_FILE = "/single_ver/main/gen_natural_store.db";
const static std::string SYNC_VALUE_SLICE_QUERY_SQL = "select count(*) from data;";
const static std::string QUERY_SQL = "select count(*) from version_data where key = ?;";
const static std::string MULTI_KEY_QUERY_SQL = "select count(*) from version_data where key = ? or key = ?;";

typedef std::chrono::time_point<std::chrono::steady_clock, std::chrono::microseconds> microClock_type;

// place some const values for testcases in namespace.
namespace DistributedDBDataGenerator {
const std::string STORE_ID = "";
const std::string STORE_ID_1 = "STORE_ID_1";
const std::string STORE_ID_2 = "STORE_ID_2";
const std::string STORE_ID_3 = "STORE_ID_3";
const std::string STORE_ID_4 = "STORE_ID_4";
const std::string STORE_ID_5 = "STORE_ID_5";
const std::string STORE_ID_6 = "STORE_ID_6";
const std::string STORE_ID_7 = "STORE_ID_7";
const std::string STORE_ID_8 = "STORE_ID_8";
const std::string STORE_ID_9 = "STORE_ID_9";
const std::string STORE_ID_10 = "STORE_ID_10";
const std::string SCHEMA_STORE_ID_11 = "SCHEMA_STORE_ID_11";
const std::string STORE_ID_PERFORM = "STORE_ID_PERFORM";
const static std::string STORE_ID_SYNC_1 = "SYNC1";
const static std::string STORE_ID_SYNC_2 = "SYNC2";
const static std::string STORE_ID_SYNC_3 = "SYNC3";
const static std::string STORE_ID_SYNC_4 = "SYNC4";
const static std::string STORE_ID_SYNC_5 = "SYNC5";
const static std::string STORE_ID_SYNC_6 = "SYNC6";
const static std::string STORE_ID_SYNC_7 = "SYNC7";
const static std::string STORE_ID_SYNC_8 = "SYNC8";
const static std::string STORE_ID_SYNC_9 = "SYNC9";
const static std::string STORE_ID_SYNC_10 = "SYNC10";

const std::string APP_ID = "APP_ID";
const std::string APP_ID_1 = "APP_ID_1";
const std::string APP_ID_2 = "APP_ID_2";
const std::string APP_ID_3 = "APP_ID_3";
const std::string APP_ID_4 = "APP_ID_4";
const std::string APP_ID_5 = "APP_ID_5";
const std::string APP_ID_6 = "APP_ID_6";
const std::string APP_ID_PERFORM = "APP_ID_PERFORM";
const std::string APP_ID_NB_1 = "APP_ID_NB_1";
const std::string APP_ID_NB_2 = "APP_ID_NB_2";
const std::string APP_ID_LOCAL_1 = "APP_ID_LOCAL_1";

const std::string USER_ID = "USER_ID";
const std::string USER_ID_1 = "USER_ID_1";
const std::string USER_ID_2 = "USER_ID_2";
const std::string USER_ID_3 = "USER_ID_3";
const std::string USER_ID_4 = "USER_ID_4";
const std::string USER_ID_5 = "USER_ID_5";
const std::string USER_ID_6 = "USER_ID_6";
const std::string USER_ID_PERFORM = "USER_ID_PERFORM";

const std::vector<uint8_t> K1_HASH_KEY = { 0xA2, 0xAB, 0x19, 0x59, 0xC1, 0xC3, 0xBF, 0xA2, 0x95, 0xB0, 0xFC, 0x90,
    0x19, 0x93, 0x78, 0x27, 0x2D, 0xB7, 0x6B, 0x45};
const std::vector<uint8_t> K2_HASH_KEY = { 0xBF, 0xEB, 0x73, 0x4D, 0x2E, 0xB5, 0xD0, 0x91, 0x51, 0x45, 0xC1, 0x86,
    0x12, 0x48, 0x75, 0x7D, 0x4F, 0xD3, 0x2B, 0xC2 };
const std::vector<uint8_t> K3_HASH_KEY = { 0xB5, 0x32, 0xA5, 0x44, 0x0D, 0xD8, 0x42, 0x2D, 0x9D, 0x5F, 0x8D, 0x99,
    0x9B, 0x31, 0x06, 0x87, 0xD4, 0xA2, 0xFE, 0xD9 };
const std::vector<uint8_t> K4_HASH_KEY = { 0x5E, 0xF8, 0x76, 0x6D, 0xE9, 0x35, 0x32, 0x44, 0x24, 0xB5, 0x63, 0xAA,
    0x3E, 0xB0, 0xC7, 0x46, 0x6B, 0x29, 0x3C, 0x94 };
const std::vector<uint8_t> NULL_HASH_KEY = {};
const DistributedDB::Key KEY_1 = { 'k', '1' };
const DistributedDB::Key KEY_2 = { 'k', '2' };
const DistributedDB::Key KEY_3 = { 'k', '3' };
const DistributedDB::Key KEY_4 = { 'k', '4' };
const DistributedDB::Key KEY_5 = { 'k', '5' };
const DistributedDB::Key KEY_6 = { 'k', '6' };
const DistributedDB::Key KEY_7 = { 'k', '7' };
const DistributedDB::Key KEY_8 = { 'k', '8' };
const DistributedDB::Key KEY_9 = { 'k', '9' };
const DistributedDB::Key KEY_10 = { 'k', '1', '0' };
const DistributedDB::Key KEY_6_BYTE = { 'k', 'k', 'k', 'k', 'k', '1' };
const DistributedDB::Key KEY_A_1 = { 'a', 'b', 'c' };
const DistributedDB::Key KEY_A_2 = { 'a', 'b', 'c', 'd', 'a', 's', 'd' };
const DistributedDB::Key KEY_A_3 = { 'a', 'b', 'c', 'd', 's' };
const DistributedDB::Key KEY_BIG_1 = { 'b', 'i', 'g', '1' };

const std::vector<uint8_t> NULL_K1 = {};
const DistributedDB::Key KEY_EMPTY = { };
const DistributedDB::Key KEY_K = { 'k' };
const DistributedDB::Key KEY_A = { 'a' };
const DistributedDB::Key OK_KEY_1 = { 'o', 'k' };

const DistributedDB::Value VALUE_1 = { 'v', '1' };
const DistributedDB::Value VALUE_2 = { 'v', '2' };
const DistributedDB::Value VALUE_3 = { 'v', '3' };
const DistributedDB::Value VALUE_4 = { 'v', '4' };
const DistributedDB::Value VALUE_5 = { 'v', '5' };
const DistributedDB::Value VALUE_6 = { 'v', '6' };
const DistributedDB::Value VALUE_7 = { 'v', '7' };
const DistributedDB::Value VALUE_8 = { 'v', '8' };
const DistributedDB::Value VALUE_9 = { 'v', '9' };
const DistributedDB::Value VALUE_10 = { 'v', '1', '0' };
const DistributedDB::Value VALUE_A_1 = { 'a', '1' };
const DistributedDB::Value VALUE_A_2 = { 'a', '2' };
const DistributedDB::Value VALUE_A_3 = { 'a', '3' };
const DistributedDB::Value VALUE_EMPTY = { };
const DistributedDB::Value OK_VALUE_1 = { 'o', 'k' };

const std::vector<uint8_t> IMAGE_KEY_PRE = {'a', 'l', 'b', 'u', 'm', '_'};
const std::vector<uint8_t> IMAGE_FILE_KEY_PRE = {'f', 'i', 'l', 'e', '_'};
const std::string IMAGE_VALUE_PRE = {"\"_id\":23,\"local_media_id\":0," \
    "\"_data\":\"/storage/emulated/0/Pictures/.Gallery2/recycle/GF6DA7BR\"," \
    "\"_size\":427460,\"date_added\":1518606965,\"date_modified\":1519460678," \
    "\"mime_type\":\"image/jpeg\",\"title\":\"MagazinePic-05-2.3.001-bigpicture_05_4\"," \
    "\"description\":\"\",\"_display_name\":\"MagazinePic-05-2.3.001-bigpicture_05_4.jpg\"," \
    "\"orientation\":0,\"latitude\":0,\"longitude\":0,\"datetaken\":1514792938000," \
    "\"bucket_id\":771419238,\"bucket_display_name\":\"MagazineUnlock\",\"duration\":0," \
    "\"resolution\":\"1440x2560\",\"media_type\":1,\"storage_id\":65537,\"width\":1440," \
    "\"height\":2560,\"is_hdr\":0,\"is_hw_privacy\":0,\"hw_voice_offset\":0,\"is_hw_favorite\":0," \
    "\"hw_image_refocus\":0,\"is_hw_burst\":0,\"hw_rectify_offset\":0,\"contenturi\":\"\"," \
    "\"hash\":\"e46cf1bb4773421fbded2e2583fe7130\",\"special_file_list\":0,\"special_file_type\":0," \
    "\"special_file_offset\":0,\"relative_bucket_id\":1793967153,\"albumId\":\"default-album-3\",\"fileType\":1," \
    "\"fileId\":0,\"videoThumbId\":0,\"thumbId\":0,\"lcdThumbId\":0,\"thumbType\":3,\"localThumbPath\":\"\"," \
    "\"localBigThumbPath\":\"\",\"expand\":\"\",\"showDateToken\":1514792938000,\"visit_time\":0," \
    "\"last_update_time\":1519461225861,\"source\":\"\",\"geo_code\":0,\"location_key\":104473884060," \
    "\"story_id\":0,\"story_cluster_state\":\"todo\",\"search_data_status\":0,\"category_id\":-2," \
    "\"portrait_id\":0,\"portrait_cluster_state\":\"todo\",\"dirty\":0,\"recycleFlag\":2," \
    "\"recycledTime\":1519550100614,\"sourcePath\":\"/storage/emulated/0/MagazineUnlock/MagazinePic\"," \
    "\"sourceFileName\":\"MagazinePic-05-2.3.001-bigpict\",\"garbage\":0,\"uniqueId\":0,\"localKey\":\"\"," \
    "\"picture_score\":99,\"cam_perception\":\"\",\"cam_exif_flag\":1,\"sync_status\":0," \
    "\"album_name\":\".MagazineUnlock\",\"ocr_status\":0"};

const DistributedDB::Key KEY_SEARCH_0 = KEY_A;
const DistributedDB::Key KEY_SEARCH = { 'a', 'b' };
const DistributedDB::Key KEY_SEARCH_2 = { 'a', 'b', 'c', 'd', 'e' };
const std::vector<uint8_t> K_SEARCH_3 = { 'b', 'i', 'g'};
const DistributedDB::Key KEY_SEARCH_3 = { 'b', 'i', 'g'};
const DistributedDB::Key KEY_SEARCH_4 = { 'k' };
const std::vector<uint8_t> K_SEARCH_5 = { 'k', 'k', 'k', 'k', 'k', 'k', 'k', 'k', 'k', 'k' };
const DistributedDB::Key KEY_SEARCH_6 = { 'k', 'k', 'k', 'k', 'k', 'k', 'k', 'k', 'k', 'k', '6' };
const DistributedDB::Key PERFORMANCEKEY = { 'p', 'e', 'r' };
const DistributedDB::Key KEY_CONS_1 = { 'c', 'o', 'n', 's', '1' };
const DistributedDB::Key KEY_CONS_2 = { 'c', 'o', 'n', 's', '2' };
const DistributedDB::Key KEY_BATCH_CONS_1 = { 'r', 'e', 's', 'u', 'l', 't', '1' };
const DistributedDB::Key KEY_BATCH_CONS_2 = { 'r', 'e', 's', 'u', 'l', 't', '2' };
const DistributedDB::Key KEY_BATCH_CONS_3 = { 'r', 'e', 's', 'u', 'l', 't', '3' };
const std::vector<DistributedDB::Key> KEYS_1 = { KEY_1, KEY_2 };
const DistributedDB::Entry ENTRY_1 = { .key = KEY_1, .value = VALUE_1 };
const DistributedDB::Entry ENTRY_2 = { .key = KEY_2, .value = VALUE_2 };
const DistributedDB::Entry ENTRY_3 = { .key = KEY_3, .value = VALUE_3 };
const DistributedDB::Entry ENTRY_4 = { .key = KEY_4, .value = VALUE_4 };
const DistributedDB::Entry ENTRY_5 = { .key = KEY_5, .value = VALUE_5 };
const DistributedDB::Entry ENTRY_6 = { .key = KEY_6, .value = VALUE_6 };
const DistributedDB::Entry ENTRY_7 = { .key = KEY_7, .value = VALUE_7 };
const DistributedDB::Entry ENTRY_8 = { .key = KEY_8, .value = VALUE_8 };
const DistributedDB::Entry ENTRY_9 = { .key = KEY_9, .value = VALUE_9 };
const DistributedDB::Entry ENTRY_10 = { .key = KEY_10, .value = VALUE_10 };
const DistributedDB::Entry ENTRY_1_2 = { .key = KEY_1, .value = VALUE_2 };
const DistributedDB::Entry ENTRY_2_3 = { .key = KEY_2, .value = VALUE_3 };
const DistributedDB::Entry ENTRY_3_1 = { .key = KEY_3, .value = VALUE_1 };
const DistributedDB::Entry ENTRY_4_5 = { .key = KEY_4, .value = VALUE_5 };
const DistributedDB::Entry ENTRY_5_6 = { .key = KEY_5, .value = VALUE_6 };
const DistributedDB::Entry ENTRY_2_1 = { .key = KEY_2, .value = VALUE_1 };
const DistributedDB::Entry ENTRY_1_3 = { .key = KEY_1, .value = VALUE_3 };
const DistributedDB::Entry ENTRY_2_4 = { .key = KEY_2, .value = VALUE_4 };
const DistributedDB::Entry ENTRY_1_4 = { .key = KEY_1, .value = VALUE_4 };
const DistributedDB::Entry ENTRY_3_4 = { .key = KEY_3, .value = VALUE_4 };
const DistributedDB::Entry ENTRY_1_NULL = { .key = KEY_1, .value = VALUE_EMPTY };
const DistributedDB::Entry ENTRY_2_NULL = { .key = KEY_2, .value = VALUE_EMPTY };
const DistributedDB::Entry ENTRY_3_NULL = { .key = KEY_3, .value = VALUE_EMPTY };
const DistributedDB::Entry ENTRY_NULL_1 = { .key = KEY_EMPTY, .value = VALUE_2 };
const DistributedDB::Entry ENTRY_NULL_2 = { .key = KEY_EMPTY, .value = VALUE_3 };
const DistributedDB::Entry ENTRY_NULL_3 = { .key = KEY_EMPTY, .value = VALUE_1 };
const DistributedDB::Entry ENTRY_NULL = { .key = KEY_EMPTY, .value = VALUE_EMPTY };
const DistributedDB::Entry ENTRY_A_1 = { .key = KEY_A_1, .value = VALUE_A_1 };
const DistributedDB::Entry ENTRY_A_2 = { .key = KEY_A_2, .value = VALUE_A_2 };
const DistributedDB::Entry ENTRY_A_3 = { .key = KEY_A_3, .value = VALUE_A_3 };
const DistributedDB::Entry ENTRY_A_1_2 = { .key = KEY_A_1, .value = VALUE_A_2 };
const DistributedDB::Entry ENTRY_A_2_3 = { .key = KEY_A_2, .value = VALUE_A_3 };
const DistributedDB::Entry ENTRY_A_3_1 = { .key = KEY_A_3, .value = VALUE_A_1 };

const DistributedDB::CipherPassword NULL_PASSWD;
const std::vector<uint8_t> NULL_PASSWD_VECTOR = {};
const std::vector<uint8_t> PASSWD_VECTOR_1 = {'P', 'a', 's', 's', 'w', 'o', 'r', 'd', '@', '1', '2', '3'};
const std::vector<uint8_t> PASSWD_VECTOR_2 = {'P', 'a', 's', 's', 'w', 'o', 'r', 'd', '@', 'c', 'o', 'm'};
const std::vector<uint8_t> FILE_PASSWD_VECTOR_1 = {'F', 'i', 'l', 'e', 'p', 'a', 's', 's', 'w', 'd', '1'};
const std::vector<uint8_t> FILE_PASSWD_VECTOR_2 = {'F', 'i', 'l', 'e', 'p', 'a', 's', 's', 'w', 'd', '2'};

void GenerateRecord(unsigned int keyNo, DistributedDB::Entry &entry, std::vector<uint8_t> keyPrifix = { 'k' });

void GenerateCharSet(std::vector<uint8_t> &charSet);

void GenerateAlphaNumUnderlineCharSet(std::vector<uint8_t> &charSet);

void GenerateSpecialCharSet(std::vector<uint8_t> &charSet);

void GenerateFixedLenRandString(unsigned int neededLen, RandType randType, std::string &genString);

void GenerateRandRecord(DistributedDB::Entry &entry, EntrySize &entrySize, unsigned int keyNo);

void GenerateRecords(unsigned int recordNum, unsigned int start,
    std::vector<DistributedDB::Key> &allKeys, std::vector<DistributedDB::Entry> &entriesBatch,
    std::vector<uint8_t> keyPrifix = { 'k' });
void GenerateMaxBigRecord(unsigned int keyNo, DistributedDB::Entry &entry,
    const std::vector<uint8_t> &keyPrefix, unsigned int num);
bool GenerateMaxBigRecords(unsigned int recordNum, unsigned int start,
    std::vector<DistributedDB::Key> &allKeys, std::vector<DistributedDB::Entry> &entriesBatch);

void GenerateTenThousandRecords(unsigned int recordNum, unsigned int start,
    std::vector<DistributedDB::Key> &allKeys, std::vector<DistributedDB::Entry> &entriesBatch);

void GenerateNormalAsciiRecords(DistributedDB::Entry &entry);

void GenerateFullAsciiRecords(DistributedDB::Entry &entry);

void GenerateBiggistKeyRecords(DistributedDB::Entry &entry);

DistributedDB::Entry GenerateFixedLenKVRecord(unsigned int serialNo,
    unsigned int keyLen, uint8_t keyFilledChr,
    unsigned int valueLen, uint8_t valueFilledChr);

void GenerateFixedRecords(std::vector<DistributedDB::Entry> &entries,
    std::vector<DistributedDB::Key> &allKeys,
    int recordNum, unsigned int keySize, unsigned int valSize);

void GenerateOneRecordForImage(int entryNo, const EntrySize &entrySize,
    const std::vector<uint8_t> &keyPrefix, const std::vector<uint8_t> &val, DistributedDB::Entry &entry);
void GenerateRecordsForImage(std::vector<DistributedDB::Entry> &entries, EntrySize &entrySize,
    int num, std::vector<uint8_t> keyPrefix = {'k'}, std::vector<uint8_t> val = {'v'});

void GenerateAppointPrefixAndSizeRecord(int recordNo, const EntrySize &entrySize,
    const std::vector<uint8_t> &keyPrefix, const std::vector<uint8_t> &valPrefix, DistributedDB::Entry &entry);
void GenerateAppointPrefixAndSizeRecords(std::vector<DistributedDB::Entry> &entries, const EntrySize &entrySize,
    int num, const std::vector<uint8_t> &keyPrefix = {'k'}, const std::vector<uint8_t> &valPrefix = {'v'});

int GetRandInt(const int randMin, const int randMax);

void GenerateFixedLenRandRecords(std::vector<DistributedDB::Entry> &entries,
    std::vector<DistributedDB::Key> &allKeys,
    int recordNum, unsigned int keySize, unsigned int valSize);

const std::string GetDbType(const int type);
void GenerateRandomRecords(std::vector<DistributedDB::Entry> &entries, EntrySize entrySize, int num);
void GetLongSchemaDefine(LongDefine &param, std::string &longDefine);
const std::string SpliceToSchema(const std::string &version, const std::string &mode,
    const std::string &define, const std::string &index = "", const std::string &skipSize = "");
std::vector<std::string> GetValidSchema(Schema &validSchema, bool hasIndex);
std::map<int, std::vector<std::string>> GetInvalidSchema(Schema &invalidSchema, Schema &validSchema, bool hasIndex);
} // DistributedDBDataGenerator
#endif // DISTRIBUTED_DB_MODULE_TEST_TYPES_H
