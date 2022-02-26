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

#ifndef DISTRIBUTEDDATAMGR_HIVIEW_MGR_H
#define DISTRIBUTEDDATAMGR_HIVIEW_MGR_H

#include <any>
#include <map>
#include <string>
#include <unistd.h>

class FakeHivew {
public:
    static std::string GetString(const std::string &key)
    {
        usleep(INTERNAL);
        if (fakeCache_.count(key)) {
            return std::any_cast<std::string>(fakeCache_[key]);
        }
        return "";
    }

    static int GetInt(const std::string &key)
    {
        usleep(INTERNAL);
        if (fakeCache_.count(key)) {
            return std::any_cast<int>(fakeCache_[key]);
        }
        return 0;
    }

    static void Put(const std::string &key, const std::any &an)
    {
        fakeCache_.insert({key, an});
    }

    static void Clear()
    {
        fakeCache_.clear();
    }

private:
    static std::map<std::string, std::any> fakeCache_;
    static const inline int INTERNAL = 1000;
};

#endif // DISTRIBUTEDDATAMGR_HIVIEW_MGR_H
