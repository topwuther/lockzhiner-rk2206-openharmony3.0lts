/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "options.h"
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>

namespace OHOS {
namespace HDI {
const char* Options::OPT_SUPPORT_ARGS = "c:d:";
static struct option g_longOpts[] = {
    {"help", no_argument, nullptr, 'h'},
    {"version", no_argument, nullptr, 'v'},
    {"gen-c", no_argument, nullptr, 'C'},
    {"gen-cpp", no_argument, nullptr, 'P'},
    {"gen-java", no_argument, nullptr, 'J'},
    {"hash", no_argument, nullptr, 'H'},
    {"dump-ast", no_argument, nullptr, 'D'},
    {nullptr, 0, nullptr, 0}
};

Options& Options::GetInstance()
{
    static Options option;
    return option;
}

Options& Options::Parse(int argc, char** argv)
{
    program_ = argv[0];
    opterr = 1;
    int op = 0;
    int optIndex = 0;

    while ((op = getopt_long(argc, argv, OPT_SUPPORT_ARGS, g_longOpts, &optIndex)) != OPT_END) {
        switch (op) {
            case 'c':
                doCompile_ = true;
                sourceFilePath_ = optarg;
                break;
            case 'd':
                doOutDir_ = true;
                generationDirectory_ = optarg;
                break;
            case 'h':
                doShowUsage_ = true;
                break;
            case 'v':
                doShowVersion_ = true;
                break;
            case 'C':
                doGenerateCode_ = true;
                targetLanguage_ = "c";
                break;
            case 'P':
                doGenerateCode_ = true;
                targetLanguage_ = "cpp";
                break;
            case 'J':
                doGenerateCode_ = true;
                targetLanguage_ = "java";
                break;
            case 'H':
                doGetHashKey_ = true;
                break;
            case 'D':
                doDumpAST_ = true;
                break;
            case '?':
            default:
                doShowUsage_ = true;
                break;
        }
    }
    CheckOptions();
    return *this;
}

void Options::CheckOptions()
{
    if (doShowUsage_ || doShowVersion_) {
        return;
    }

    if (doCompile_) {
        if (!doGetHashKey_ && !doDumpAST_ && !doGenerateCode_ && !doOutDir_) {
            errors_.push_back(String::Format("%s: nothing to do.", program_.string()));
            return;
        }

        if (!doGenerateCode_ && doOutDir_) {
            errors_.push_back(String::Format("%s: no target language.", program_.string()));
            return;
        }

        if (doGenerateCode_ && !doOutDir_) {
            errors_.push_back(String::Format("%s: no out directory.", program_.string()));
            return;
        }
    } else {
        if (doGetHashKey_ || doDumpAST_ || doGenerateCode_ || doOutDir_) {
            errors_.push_back(String::Format("%s: no '-c' option.", program_.string()));
            return;
        }
    }
}

void Options::ShowErrors() const
{
    for (auto error : errors_) {
        printf("%s\n", error.string());
    }
    printf("Use \"--help\" to show usage.\n");
}

void Options::ShowVersion() const
{
    printf("HDI-GEN: %d.%d\n"
          "Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.\n\n",
           VERSION_MAJOR, VERSION_MINOR);
}

void Options::ShowUsage() const
{
    printf("Compile a .idl file and generate C/C++ and Java codes.\n"
           "Usage: idl [options] file\n"
           "Options:\n"
           "  --help            Display command line options\n"
           "  --version         Display toolchain version information\n"
           "  --hash            Display hash key of the idl file\n"
           "  --dump-ast        Display the AST of the compiled file\n"
           "  -c                Compile the .idl file\n"
           "  --gen-c           Generate C codes\n"
           "  --gen-cpp         Generate C++ codes\n"
           "  --gen-java        Generate Java codes\n"
           "  -d <directory>    Place generated codes into <directory>\n");
}
} // namespace HDI
} // namespace OHOS