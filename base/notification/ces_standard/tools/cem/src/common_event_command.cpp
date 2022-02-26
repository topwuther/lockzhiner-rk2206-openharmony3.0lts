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

#include "common_event_command.h"

#include <getopt.h>
#include "common_event_manager.h"
#include "common_event.h"
#include "singleton.h"
#include "event_log_wrapper.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace EventFwk {
namespace {
const std::string SHORT_OPTIONS = "he:asoc:d:";
const struct option LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"all", no_argument, nullptr, 'a'},
    {"event", required_argument, nullptr, 'e'},
    {"sticky", no_argument, nullptr, 's'},
    {"ordered", no_argument, nullptr, 'o'},
    {"code", required_argument, nullptr, 'c'},
    {"data", required_argument, nullptr, 'd'},
    {"ebool", required_argument, nullptr, 0},
    {"ebyte", required_argument, nullptr, 0},
    {"echar", required_argument, nullptr, 0},
    {"eint", required_argument, nullptr, 0},
    {"edouble", required_argument, nullptr, 0},
    {"efloat", required_argument, nullptr, 0},
    {"elong", required_argument, nullptr, 0},
    {"eshort", required_argument, nullptr, 0},
    {"estring", required_argument, nullptr, 0},
};
}  // namespace

CommonEventManagerShellCommand::CommonEventManagerShellCommand(int argc, char *argv[])
    : ShellCommand(argc, argv, TOOL_NAME)
{}

ErrCode CommonEventManagerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&CommonEventManagerShellCommand::RunAsHelpCommand, this)},
        {"publish", std::bind(&CommonEventManagerShellCommand::RunAsPublishCommand, this)},
        {"dump", std::bind(&CommonEventManagerShellCommand::RunAsDumpCommand, this)},
    };

    return OHOS::ERR_OK;
}

ErrCode CommonEventManagerShellCommand::CreateMessageMap()
{
    messageMap_ = {};

    return OHOS::ERR_OK;
}

ErrCode CommonEventManagerShellCommand::init()
{
    EVENT_LOGI("enter");

    ErrCode result = OHOS::ERR_OK;

    if (!commonEventPtr_) {
        commonEventPtr_ = DelayedSingleton<CommonEvent>::GetInstance();
    }

    if (!commonEventPtr_) {
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode CommonEventManagerShellCommand::RunAsHelpCommand()
{
    EVENT_LOGI("enter");

    resultReceiver_.append(HELP_MSG);

    return OHOS::ERR_OK;
}

ErrCode CommonEventManagerShellCommand::RunAsPublishCommand()
{
    EVENT_LOGI("enter");

    ErrCode result = OHOS::ERR_OK;

    int option = -1;
    int counter = 0;

    bool isSticky = false;
    bool isOrdered = false;

    std::string action = "";
    int code = 0;
    std::string data = "";

    std::string boolKey = "";
    bool boolParam = false;

    std::string byteKey = "";
    OHOS::AAFwk::byte byteParam = 0;

    std::string charKey = "";
    zchar charParam = 0;

    std::string intKey = "";
    int intParam = 0;

    std::string doubleKey = "";
    double doubleParam = 0;

    std::string floatKey = "";
    float floatParam = 0;

    std::string longKey = "";
    long longParam = 0;

    std::string shortKey = "";
    short shortParam = 0;

    std::string stringKey = "";
    std::string stringParam = "";

    while (true) {
        counter++;

        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);

        EVENT_LOGI("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        for (int i = 0; i < argc_; i++) {
            EVENT_LOGI("argv_[%{public}d]: %{public}s", i, argv_[i]);
        }

        if (option == -1) {
            if (counter == 1) {
                // When scanning the first argument
                if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                    // 'cem publish' with no option: cem publish
                    // 'cem publish' with a wrong argument: cem publish xxx
                    EVENT_LOGI("'cem publish' with no option.");

                    resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");

                    result = OHOS::ERR_INVALID_VALUE;
                }
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'e': {
                    // 'cem publish -e' with no argument: cem publish -e
                    // 'cem publish --event' with no argument: cem publish --event
                    EVENT_LOGI("'cem publish -e' with no argument.");

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'c': {
                    // 'cem publish -e <name> -c' with no argument: cem publish -e <name> -c
                    // 'cem publish --event <name> -c' with no argument: cem publish --event -c
                    EVENT_LOGI("'cem publish -e <name> -c' with no argument.");

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'd': {
                    // 'cem publish -e <name> -d' with no argument: cem publish -e <name> -d
                    // 'cem publish --event <name> -d' with no argument: cem publish --event -d
                    EVENT_LOGI("'cem publish -e <name> -d' with no argument.");

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 0: {
                    std::string longOption = argv_[optind - 1];

                    if (longOption == "--ebool" || longOption == "--ebyte" || longOption == "--echar" ||
                        longOption == "--eint" || longOption == "--edouble" || longOption == "--efloat" ||
                        longOption == "--elong" || longOption == "--eshort" || longOption == "--estring") {
                        resultReceiver_.append("error: option ");
                        resultReceiver_.append("requires a value at least.\n");
                    } else {
                        // 'cem publish' with an unknown option: cem publish --x
                        // 'cem publish' with an unknown option: cem publish --xxx
                        std::string unknownOption = "";
                        std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                        EVENT_LOGI("'cem publish' with an unknown option.");

                        resultReceiver_.append(unknownOptionMsg);
                    }

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'cem publish' with an unknown option: cem publish -x
                    // 'cem publish' with an unknown option: cem publish -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    EVENT_LOGI("'cem publish' with an unknown option.");

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'cem publish -h'
                // 'cem publish --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'e': {
                // 'cem publish -e <name>'
                // 'cem publish --event <name>'
                action = optarg;
                break;
            }
            case 's': {
                // 'cem publish -e <name> -s '
                // 'cem publish -e <name> --sticky'
                isSticky = true;
                break;
            }
            case 'o': {
                // 'cem publish -e <name> -o '
                // 'cem publish -e <name> --ordered'
                isOrdered = true;
                break;
            }
            case 'c': {
                // 'cem publish -e <name> -c 1024 '
                // 'cem publish -e <name> --code 1024'
                code = atoi(optarg);
                break;
            }
            case 'd': {
                // 'cem publish -e <name> -d 1024 '
                // 'cem publish -e <name> --data 1024'
                data = optarg;
                break;
            }
            case 0: {
                int optionIndex = optind - 2;

                std::string optionString = argv_[optionIndex];

                std::string optionKey = "";
                if (argv_[optionIndex + 1]) {
                    optionKey = argv_[optionIndex + 1];
                }

                std::string optionParam = "";
                if (argv_[optionIndex + 2]) {
                    optionParam = argv_[optionIndex + 2];
                }

                switch (optopt) {
                    case '?': {
                        if (optionString == "--ebool") {
                            boolKey = optionKey;

                            if (optionParam == "true") {
                                boolParam = true;
                            } else {
                                boolParam = false;
                            }
                        } else if (optionString == "--ebyte") {
                            byteKey = optionKey;

                            byteParam = optionParam.size() ? optionParam.at(0) : 0;
                        } else if (optionString == "--echar") {
                            charKey = optionKey;

                            charParam = optionParam.size() ? optionParam.at(0) : 0;
                        } else if (optionString == "--eint") {
                            intKey = optionKey;

                            intParam = atoi(optionParam.c_str());
                        } else if (optionString == "--edouble") {
                            doubleKey = optionKey;

                            doubleParam = atof(optionParam.c_str());
                        } else if (optionString == "--efloat") {
                            floatKey = optionKey;

                            floatParam = atof(optionParam.c_str());
                        } else if (optionString == "--elong") {
                            longKey = optionKey;

                            longParam = atol(optionParam.c_str());
                        } else if (optionString == "--eshort") {
                            shortKey = optionKey;

                            shortParam = atoi(optionParam.c_str());
                        } else if (optionString == "--estring") {
                            stringKey = optionKey;

                            stringParam = optionParam;
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        if (resultReceiver_ == "" && action.size() == 0) {
            // 'cem publish ...' with no event option
            EVENT_LOGI("'cem publish' with no event option.");

            resultReceiver_.append(HELP_MSG_NO_EVENT_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_PUBLISH);
    } else {
        /* Publish */

        // make a want
        Want want;
        want.SetAction(action);

        // set bool param of the want if necessary
        if (boolKey.size() > 0) {
            EVENT_LOGI("boolParam: %{public}d", boolParam);

            want.SetParam(boolKey, boolParam);
        }

        // set byte param of the want if necessary
        if (byteKey.size() > 0) {
            EVENT_LOGI("byteParam: %{public}d", byteParam);

            want.SetParam(byteKey, byteParam);
        }

        // set char param of the want if necessary
        if (charKey.size() > 0) {
            EVENT_LOGI("charParam: %{public}d", charParam);

            want.SetParam(charKey, charParam);
        }

        // set int param of the want if necessary
        if (intKey.size() > 0) {
            EVENT_LOGI("intParam: %{public}d", intParam);

            want.SetParam(intKey, intParam);
        }

        // set double param of the want if necessary
        if (doubleKey.size() > 0) {
            EVENT_LOGI("doubleParam: %{public}f", doubleParam);

            want.SetParam(doubleKey, doubleParam);
        }

        // set float param of the want if necessary
        if (floatKey.size() > 0) {
            EVENT_LOGI("floatParam: %{public}f", floatParam);

            want.SetParam(floatKey, floatParam);
        }

        // set long param of the want if necessary
        if (longKey.size() > 0) {
            EVENT_LOGI("longParam: %{public}ld", longParam);

            want.SetParam(longKey, longParam);
        }

        // set short param of the want if necessary
        if (shortKey.size() > 0) {
            EVENT_LOGI("shortParam: %{public}d", shortParam);

            want.SetParam(shortKey, shortParam);
        }

        // set string param of the want if necessary
        if (stringKey.size() > 0) {
            EVENT_LOGI("stringParam: %{public}s", stringParam.c_str());

            want.SetParam(stringKey, stringParam);
        }

        // make common event data
        CommonEventData commonEventData;
        commonEventData.SetWant(want);

        // set code
        commonEventData.SetCode(code);

        // set data
        commonEventData.SetData(data);

        // make publish info
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSticky(isSticky);
        publishInfo.SetOrdered(isOrdered);

        // publish the common event
        std::shared_ptr<CommonEventSubscriber> subscriber = nullptr;
        bool publishResult = commonEventPtr_->PublishCommonEvent(commonEventData, publishInfo, subscriber);
        if (publishResult) {
            resultReceiver_ = STRING_PUBLISH_COMMON_EVENT_OK + "\n";
        } else {
            resultReceiver_ = STRING_PUBLISH_COMMON_EVENT_NG + "\n";
        }
    }

    return result;
}

ErrCode CommonEventManagerShellCommand::RunAsDumpCommand()
{
    EVENT_LOGI("enter");

    ErrCode result = OHOS::ERR_OK;
    std::vector<std::string> dumpResults;

    std::string action = "";

    int option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);

    EVENT_LOGI("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

    if (optind < 0 || optind > argc_) {
        return OHOS::ERR_INVALID_VALUE;
    }

    for (int i = 0; i < argc_; i++) {
        EVENT_LOGI("argv_[%{public}d]: %{public}s", i, argv_[i]);
    }

    switch (option) {
        case 'h': {
            // 'cem dump -h'
            // 'cem dump --help'
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }
        case 'a': {
            // 'cem dump -a'
            // 'cem dump --all'
            break;
        }
        case 'e': {
            // 'cem dump -e <name>'
            // 'cem dump --event <name>'
            action = optarg;
            break;
        }
        case '?': {
            switch (optopt) {
                case 'e': {
                    // 'cem dump -e' with no argument: cem dump -e
                    // 'cem dump --event' with no argument: cem dump --event
                    EVENT_LOGI("'cem dump -s' with no argument.");

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 0: {
                    // 'cem dump' with an unknown option: cem dump --x
                    // 'cem dump' with an unknown option: cem dump --xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    EVENT_LOGI("'cem dump' with an unknown option.");

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'cem dump' with an unknown option: cem dump -x
                    // 'cem dump' with an unknown option: cem dump -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    EVENT_LOGI("'cem dump' with an unknown option.");

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }
        default: {
            if (strcmp(argv_[optind], cmd_.c_str()) == 0) {
                // 'cem dump' with no option: cem dump
                // 'cem dump' with a wrong argument: cem dump xxx
                EVENT_LOGI("'cem dump' with no option.");

                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                result = OHOS::ERR_INVALID_VALUE;
            }
            break;
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_DUMP);
    } else {
        // dump state
        bool dumpResult = commonEventPtr_->DumpState(action, dumpResults);
        if (dumpResult) {
            for (auto it : dumpResults) {
                resultReceiver_ += it + "\n";
            }
        } else {
            resultReceiver_ = STRING_DUMP_COMMON_EVENT_NG + "\n";
        }
    }

    return result;
}
}  // namespace EventFwk
}  // namespace OHOS
