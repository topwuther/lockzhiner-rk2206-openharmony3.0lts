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
#ifndef FUZZ_CONFIG_PARSER_H
#define FUZZ_CONFIG_PARSER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace OHOS {

const std::string FUZZ_TEST_CONFIG_FILE_PATH{"/data/fuzztestconfig/config.json"};

const std::string FUZZ_TEST_MAIN_LOOP_KEY{"flag"};

struct FuzzTestData {
    int32_t mainLoopFlag{0};
    std::vector<std::string> methodVec{};
};

class FuzzConfigParser {
public:
    void ParseFromFile4FuzzTest(const std::string &path, FuzzTestData &ftd)
    {
        std::cout << __func__ << std::endl;
        if (path.empty()) {
            std::cout << __FUNCTION__ << " invalid file path, check!" << std::endl;
            return;
        }

        nlohmann::json jsonObj;
        std::ifstream(path) >> jsonObj;
        std::cout << path;
        for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
            if (!it.key().compare(FUZZ_TEST_MAIN_LOOP_KEY)) {
                ftd.mainLoopFlag = it.value();
                continue;
            }

            auto className = it.key();
            if (it->is_structured()) {
                for (auto itm = it->begin(); itm != it->end(); ++itm) {
                    auto methodName = itm.key();

                    if (!(it->is_structured() && (it->size() != 0))) {
                        ftd.methodVec.push_back(className + methodName);
                        continue;
                    }

                    std::string param{};
                    for (auto itp = itm->begin(); itp != itm->end(); ++itp) {
                        auto tp = itp.value();
                        param += tp;
                    }
                    ftd.methodVec.push_back(className + methodName + param);
                }
            }
        }
    }
};

}  // namespace OHOS

#endif  // FUZZ_CONFIG_PARSER_H
