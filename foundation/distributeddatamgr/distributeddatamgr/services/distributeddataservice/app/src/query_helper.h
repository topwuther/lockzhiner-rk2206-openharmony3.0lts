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

#ifndef QUERY_HELPER_H
#define QUERY_HELPER_H

#include "query.h"

namespace OHOS::DistributedKv {
class QueryHelper {
public:
    static DistributedDB::Query StringToDbQuery(const std::string &query, bool &isSuccess);
private:
    static std::string deviceId_;
    static bool hasPrefixKey_;
    static void Handle(const std::vector<std::string> &words, int &pointer,
                       const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleExtra(const std::vector<std::string> &words, int &pointer,
                            const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleEqualTo(const std::vector<std::string> &words, int &pointer,
                              const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleNotEqualTo(const std::vector<std::string> &words, int &pointer,
                                 const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleGreaterThan(const std::vector<std::string> &words, int &pointer,
                                  const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleLessThan(const std::vector<std::string> &words, int &pointer,
                               const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleGreaterThanOrEqualTo(const std::vector<std::string> &words, int &pointer,
                                           const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleLessThanOrEqualTo(const std::vector<std::string> &words, int &pointer,
                                        const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleIsNull(const std::vector<std::string> &words, int &pointer,
                             const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleIsNotNull(const std::vector<std::string> &words, int &pointer,
                             const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleIn(const std::vector<std::string> &words, int &pointer,
                         const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleNotIn(const std::vector<std::string> &words, int &pointer,
                            const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleLike(const std::vector<std::string> &words, int &pointer,
                           const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleNotLike(const std::vector<std::string> &words, int &pointer,
                              const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleAnd(const std::vector<std::string> &words, int &pointer,
                          const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleOr(const std::vector<std::string> &words, int &pointer,
                         const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleOrderByAsc(const std::vector<std::string> &words, int &pointer,
                                 const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleOrderByDesc(const std::vector<std::string> &words, int &pointer,
                                  const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleLimit(const std::vector<std::string> &words, int &pointer,
                            const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleBeginGroup(const std::vector<std::string> &words, int &pointer,
                                 const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleEndGroup(const std::vector<std::string> &words, int &pointer,
                               const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleKeyPrefix(const std::vector<std::string> &words, int &pointer,
                                const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleSetSuggestIndex(const std::vector<std::string> &words, int &pointer,
                                      const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static void HandleDeviceId(const std::vector<std::string> &words, int &pointer,
                               const int &end, bool &isSuccess, DistributedDB::Query &dbQuery);
    static int StringToInt(const std::string &word);
    static int64_t StringToLong(const std::string &word);
    static double StringToDouble(const std::string &word);
    static bool StringToBoolean(const std::string &word);
    static std::string StringToString(const std::string &word);
    static std::vector<int> GetIntegerList(const std::vector<std::string> &words, int &elementPointer, const int &end);
    static std::vector<int64_t> GetLongList(const std::vector<std::string> &words, int &elementPointer, const int &end);
    static std::vector<double> GetDoubleList(const std::vector<std::string> &words,
                                             int &elementPointer, const int &end);
    static std::vector<std::string> GetStringList(const std::vector<std::string> &words,
                                                  int &elementPointer, const int &end);
};
} // namespace OHOS::DistributedKv
#endif // QUERY_HELPER_H
