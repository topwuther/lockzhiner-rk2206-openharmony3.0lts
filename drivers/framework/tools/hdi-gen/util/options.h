/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_OPTION_H
#define OHOS_HDI_OPTION_H

#include <vector>
#include "util/string.h"

namespace OHOS {
namespace HDI {
class Options {
public:
    static Options& GetInstance();

    Options(const Options& other) = delete;
    Options operator=(const Options& other) = delete;

    Options& Parse(int argc, char** argv);

    ~Options() = default;

    inline bool DoShowUsage() const
    {
        return doShowUsage_;
    }

    inline bool DoShowVersion() const
    {
        return doShowVersion_;
    }

    inline bool DoCompile() const
    {
        return doCompile_;
    }

    inline bool DoDumpAST() const
    {
        return doDumpAST_;
    }

    inline bool DoGetHashKey() const
    {
        return doGetHashKey_;
    }

    inline bool DoGenerateCode() const
    {
        return doGenerateCode_;
    }

    inline bool HasErrors() const
    {
        return !errors_.empty();
    }

    inline String GetSourceFile() const
    {
        return sourceFilePath_;
    }

    inline String GetTargetLanguage() const
    {
        return targetLanguage_;
    }

    inline String GetGenerationDirectory() const
    {
        return generationDirectory_;
    }

    void ShowErrors() const;

    void ShowVersion() const;

    void ShowUsage() const;

private:
    Options() : program_(),
        sourceFilePath_(),
        targetLanguage_(),
        generationDirectory_(),
        illegalOptions_(),
        errors_(),
        doShowUsage_(false),
        doShowVersion_(false),
        doCompile_(false),
        doDumpAST_(false),
        doGetHashKey_(false),
        doGenerateCode_(false),
        doOutDir_(false) {}
    
    void CheckOptions();

    static const char* OPT_SUPPORT_ARGS;
    static constexpr int OPT_END = -1;

    static constexpr int VERSION_MAJOR = 0;
    static constexpr int VERSION_MINOR = 1;

    String program_;
    String sourceFilePath_;
    String targetLanguage_;
    String generationDirectory_;
    String illegalOptions_;
    std::vector<String> errors_;

    bool doShowUsage_ = false;
    bool doShowVersion_ = false;
    bool doCompile_ = false;
    bool doDumpAST_ = false;
    bool doGetHashKey_ = false;
    bool doGenerateCode_ = false;
    bool doOutDir_ = false;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDIL_OPTION_H
