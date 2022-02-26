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

#ifndef DISTRIBUTEDDB_LOG_PRINT_H
#define DISTRIBUTEDDB_LOG_PRINT_H

#include <cstdarg>
#include <cstdio>
#include <string>

namespace DistributedDB {
const std::string LOG_TAG_KV = "DistributedDB";

class Logger {
public:
    enum class Level {
        LEVEL_DEBUG,
        LEVEL_INFO,
        LEVEL_WARN,
        LEVEL_ERROR,
        LEVEL_FATAL
    };

    virtual ~Logger() {};
    static Logger *GetInstance();
    static void RegisterLogger(Logger *logger);
    static void Log(Level level, const std::string &tag, const char *func, int line, const char *format, ...);

private:
    virtual void Print(Level level, const std::string &tag, const std::string &msg) = 0;
    static void PreparePrivateLog(const char *format, std::string &outStrFormat);
    static Logger *logHandler;
    static const std::string PRIVATE_TAG;
};

#define NO_LOG(...) // No log in normal and release. Used for convenience when deep debugging
#define LOGD(...) Logger::Log(Logger::Level::LEVEL_DEBUG, LOG_TAG_KV, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOGI(...) Logger::Log(Logger::Level::LEVEL_INFO, LOG_TAG_KV, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOGW(...) Logger::Log(Logger::Level::LEVEL_WARN, LOG_TAG_KV, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOGE(...) Logger::Log(Logger::Level::LEVEL_ERROR, LOG_TAG_KV, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOGF(...) Logger::Log(Logger::Level::LEVEL_FATAL, LOG_TAG_KV, __FUNCTION__, __LINE__, __VA_ARGS__)
} // namespace DistributedDB

#endif // DISTRIBUTEDDB_LOG_PRINT_H
