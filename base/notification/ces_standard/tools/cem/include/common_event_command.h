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

#ifndef BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_CEM_INCLUDE_COMMON_EVENT_COMMAND_H
#define BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_CEM_INCLUDE_COMMON_EVENT_COMMAND_H

#include "shell_command.h"
#include "common_event.h"

namespace OHOS {
namespace EventFwk {
namespace {
const std::string TOOL_NAME = "cem";

const std::string HELP_MSG = "usage: cem <command> [<options>]\n"
                             "These are common cem commands list:\n"
                             "  help                         list available commands\n"
                             "  publish                      publish a common event with options\n"
                             "  dump                         dump the info of events\n";

const std::string HELP_MSG_PUBLISH =
    "usage: cem publish [<options>]\n"
    "options list:\n"
    "  -h, --help                                       list available commands\n"
    "  -e, --event <name> [-s, --sticky] [-o, --ordered] [-c, --code <code>] [-d, --data <data>]\n"
    "                                                   publish a common event\n";

const std::string HELP_MSG_DUMP = "usage: cem dump [<options>]\n"
                                  "options list:\n"
                                  "  -h, --help                   list available commands\n"
                                  "  -a, --all                    dump the info of all events\n"
                                  "  -e, --event <name>           dump the info of a specificed event\n";

const std::string HELP_MSG_NO_EVENT_OPTION = "error: you must specify an event name with '-e' or '--event'.";

const std::string STRING_PUBLISH_COMMON_EVENT_OK = "publish the common event successfully.";
const std::string STRING_PUBLISH_COMMON_EVENT_NG = "error: failed to publish the common event.";

const std::string STRING_DUMP_COMMON_EVENT_NG = "error: failed to dump the common event(s).";
}  // namespace

class CommonEventManagerShellCommand : public OHOS::AAFwk::ShellCommand {
public:
    CommonEventManagerShellCommand(int argc, char *argv[]);
    ~CommonEventManagerShellCommand() override
    {}

private:
    ErrCode CreateCommandMap() override;
    ErrCode CreateMessageMap() override;
    ErrCode init() override;

    ErrCode RunAsHelpCommand();
    ErrCode RunAsPublishCommand();
    ErrCode RunAsDumpCommand();

    std::shared_ptr<CommonEvent> commonEventPtr_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_CES_STANDARD_CESFWK_TOOLS_CEM_INCLUDE_COMMON_EVENT_COMMAND_H
