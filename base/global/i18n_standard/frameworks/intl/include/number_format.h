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
#ifndef OHOS_GLOBAL_I18N_NUMBER_FORMAT_H
#define OHOS_GLOBAL_I18N_NUMBER_FORMAT_H

#include "unicode/numberformatter.h"
#include "unicode/locid.h"
#include "unicode/numfmt.h"
#include "unicode/unum.h"
#include "unicode/decimfmt.h"
#include "unicode/localebuilder.h"
#include "unicode/numsys.h"
#include "unicode/measfmt.h"
#include "unicode/measunit.h"
#include "unicode/measure.h"
#include "unicode/currunit.h"
#include "unicode/fmtable.h"
#include "number_utils.h"
#include "number_utypes.h"
#include "locale_info.h"
#include <map>
#include <set>
#include <vector>

namespace OHOS {
namespace Global {
namespace I18n {
class NumberFormat {
public:
    NumberFormat(const std::vector<std::string> &localeTag, std::map<std::string, std::string> &configs);
    virtual ~NumberFormat();
    std::string Format(double number);
    void GetResolvedOptions(std::map<std::string, std::string> &map);
    std::string GetCurrency() const;
    std::string GetCurrencySign() const;
    std::string GetStyle() const;
    std::string GetNumberingSystem() const;
    std::string GetUseGrouping() const;
    std::string GetMinimumIntegerDigits() const;
    std::string GetMinimumFractionDigits() const;
    std::string GetMaximumFractionDigits() const;
    std::string GetMinimumSignificantDigits() const;
    std::string GetMaximumSignificantDigits() const;
    std::string GetLocaleMatcher() const;

private:
    icu::Locale locale;
    std::string currency;
    std::string currencySign;
    std::string currencyDisplayString;
    std::string unit;
    std::string unitDisplayString;
    std::string styleString;
    std::string numberingSystem;
    std::string useGrouping;
    std::string notationString;
    std::string signDisplayString;
    std::string compactDisplay;
    std::string minimumIntegerDigits;
    std::string minimumFractionDigits;
    std::string maximumFractionDigits;
    std::string minimumSignificantDigits;
    std::string maximumSignificantDigits;
    std::string localeBaseName;
    std::string localeMatcher;
    LocaleInfo *localeInfo = nullptr;
    icu::number::LocalizedNumberFormatter numberFormat;
    icu::number::Notation notation = icu::number::Notation::simple();
    UNumberUnitWidth unitDisplay = UNumberUnitWidth::UNUM_UNIT_WIDTH_SHORT;
    UNumberUnitWidth currencyDisplay = UNumberUnitWidth::UNUM_UNIT_WIDTH_SHORT;
    UNumberSignDisplay signDisplay = UNumberSignDisplay::UNUM_SIGN_AUTO;
    static const int MAX_UNIT_NUM = 500;
    icu::MeasureUnit unitArray[MAX_UNIT_NUM];
    static bool icuInitialized;
    static bool Init();
    static std::map<std::string, UNumberUnitWidth> unitStyle;
    static std::map<std::string, UNumberUnitWidth> currencyStyle;
    static std::map<std::string, UNumberSignDisplay> signAutoStyle;
    static std::map<std::string, UNumberSignDisplay> signAccountingStyle;
    static std::set<std::string> allValidLocales;
    static std::set<std::string> GetValidLocales();
    void ParseConfigs(std::map<std::string, std::string> &configs);
    void ParseDigitsConfigs(std::map<std::string, std::string> &configs);
    void GetDigitsResolvedOptions(std::map<std::string, std::string> &map);
    void InitProperties();
    void InitDigitsProperties();
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif