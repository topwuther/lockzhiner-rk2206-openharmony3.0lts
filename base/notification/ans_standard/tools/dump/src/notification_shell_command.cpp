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

#include "notification_shell_command.h"

#include <getopt.h>
#include <iostream>

#include "ans_inner_errors.h"
#include "iservice_registry.h"
#include "singleton.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Notification {
namespace {

static const struct option OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"active", no_argument, nullptr, 'A'},
    {"recent", no_argument, nullptr, 'R'},
    {"setRecentCount", required_argument, nullptr, 0},
    {0, 0, 0, 0},
};

static const std::string HELP_MSG = "usage: anm <command> [<options>]\n"
                                    "These are common commands list:\n"
                                    "  help                         list available commands\n"
                                    "  dump                         dump the info of notification\n";
static const std::string DUMP_HELP_MSG =
    "usage: anm dump [<options>]\n"
    "options list:\n"
    "  --help, -h                   help menu\n"
    "  --active, -A                 list all active notifications\n"
    "  --recent, -R                 list recent notifications\n"
    "  --setRecentCount <N>         set the max count of recent notification keeping in memory\n";
}  // namespace

NotificationShellCommand::NotificationShellCommand(int argc, char *argv[]) : ShellCommand(argc, argv, "anm_dump")
{}

ErrCode NotificationShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&NotificationShellCommand::RunAsHelpCommand, this)},
        {"dump", std::bind(&NotificationShellCommand::RunAsDumpCommand, this)},
    };

    return ERR_OK;
}

ErrCode NotificationShellCommand::CreateMessageMap()
{
    messageMap_ = {};

    return ERR_OK;
}

ErrCode NotificationShellCommand::init()
{
    ErrCode result = OHOS::ERR_OK;

    if (!ans_) {
        ans_ = DelayedSingleton<AnsNotification>::GetInstance();
    }

    if (!ans_) {
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode NotificationShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(HELP_MSG);
    return ERR_OK;
}

ErrCode NotificationShellCommand::RunAsDumpCommand()
{
    int ind = 0;
    int option = getopt_long(argc_, argv_, "hAR", OPTIONS, &ind);

    ErrCode ret = ERR_OK;
    std::vector<std::string> infos;

    switch (option) {
        case 'h':
            resultReceiver_.append(DUMP_HELP_MSG);
            break;
        case 'A':
            if (ans_ != nullptr) {
                ret = ans_->ShellDump("active", infos);
                resultReceiver_.append("Total:" + std::to_string(infos.size()) + "\n");
            } else {
                ret = ERR_ANS_SERVICE_NOT_CONNECTED;
            }
            break;
        case 'R':
            if (ans_ != nullptr) {
                ret = ans_->ShellDump("recent", infos);
                resultReceiver_.append("Total:" + std::to_string(infos.size()) + "\n");
            } else {
                ret = ERR_ANS_SERVICE_NOT_CONNECTED;
            }
            break;
        case 0:
            if (ans_ != nullptr) {
                ret = ans_->ShellDump(std::string(OPTIONS[ind].name) + " " + std::string(optarg), infos);
            } else {
                ret = ERR_ANS_SERVICE_NOT_CONNECTED;
            }
            break;
        default:
            std::cout << __LINE__ << " " << option << " " << ind << " " << std::endl;
            if (optarg)
                std::cout << optarg << std::endl;
            resultReceiver_.append(DUMP_HELP_MSG);
            break;
    }

    int index = 0;
    for (auto info : infos) {
        resultReceiver_.append("No." + std::to_string(index) + "\n");
        resultReceiver_.append(info);
    }

    return ret;
}

}  // namespace Notification
}  // namespace OHOS
