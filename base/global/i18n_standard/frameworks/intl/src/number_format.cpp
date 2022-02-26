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
#include "number_format.h"
#include <locale>
#include <codecvt>
#include "ohos/init_data.h"

namespace OHOS {
namespace Global {
namespace I18n {
bool NumberFormat::icuInitialized = NumberFormat::Init();

std::set<std::string> NumberFormat::allValidLocales = GetValidLocales();

std::set<std::string> NumberFormat::GetValidLocales()
{
    int32_t validCount = 1;
    const icu::Locale *validLocales = icu::Locale::getAvailableLocales(validCount);
    std::set<std::string> allValidLocales;
    for (int i = 0; i < validCount; i++) {
        allValidLocales.insert(validLocales[i].getLanguage());
    }
    return allValidLocales;
}

std::map<std::string, UNumberUnitWidth> NumberFormat::unitStyle = {
    { "long", UNumberUnitWidth::UNUM_UNIT_WIDTH_FULL_NAME },
    { "short", UNumberUnitWidth::UNUM_UNIT_WIDTH_SHORT },
    { "narrow", UNumberUnitWidth::UNUM_UNIT_WIDTH_NARROW }
};

std::map<std::string, UNumberUnitWidth> NumberFormat::currencyStyle = {
    { "symbol", UNumberUnitWidth::UNUM_UNIT_WIDTH_SHORT },
    { "code", UNumberUnitWidth::UNUM_UNIT_WIDTH_ISO_CODE },
    { "name", UNumberUnitWidth::UNUM_UNIT_WIDTH_FULL_NAME },
    { "narrowSymbol", UNumberUnitWidth::UNUM_UNIT_WIDTH_NARROW }
};

std::map<std::string, UNumberSignDisplay> NumberFormat::signAutoStyle = {
    { "auto", UNumberSignDisplay::UNUM_SIGN_AUTO },
    { "never", UNumberSignDisplay::UNUM_SIGN_NEVER },
    { "always", UNumberSignDisplay::UNUM_SIGN_ALWAYS },
    { "exceptZero", UNumberSignDisplay::UNUM_SIGN_EXCEPT_ZERO }
};

std::map<std::string, UNumberSignDisplay> NumberFormat::signAccountingStyle = {
    { "auto", UNumberSignDisplay::UNUM_SIGN_ACCOUNTING },
    { "never", UNumberSignDisplay::UNUM_SIGN_NEVER },
    { "always", UNumberSignDisplay::UNUM_SIGN_ACCOUNTING_ALWAYS },
    { "exceptZero", UNumberSignDisplay::UNUM_SIGN_ACCOUNTING_EXCEPT_ZERO }
};

NumberFormat::NumberFormat(const std::vector<std::string> &localeTags, std::map<std::string, std::string> &configs)
{
    UErrorCode status = U_ZERO_ERROR;
    auto builder = std::make_unique<icu::LocaleBuilder>();
    ParseConfigs(configs);
    for (size_t i = 0; i < localeTags.size(); i++) {
        std::string curLocale = localeTags[i];
        locale = builder->setLanguageTag(icu::StringPiece(curLocale)).build(status);
        if (allValidLocales.count(locale.getLanguage()) > 0) {
            localeInfo = new LocaleInfo(curLocale, configs);
            locale = localeInfo->GetLocale();
            localeBaseName = localeInfo->GetBaseName();
            numberFormat = icu::number::NumberFormatter::withLocale(locale);
            icu::MeasureUnit::getAvailable(unitArray, MAX_UNIT_NUM, status);
            break;
        }
    }
    if (localeInfo == nullptr) {
        localeInfo = new LocaleInfo(icu::Locale::getDefault().getBaseName(), configs);
        locale = localeInfo->GetLocale();
        localeBaseName = localeInfo->GetBaseName();
        numberFormat = icu::number::NumberFormatter::withLocale(locale);
        icu::MeasureUnit::getAvailable(unitArray, MAX_UNIT_NUM, status);
    }
    InitProperties();
}

NumberFormat::~NumberFormat()
{
    if (localeInfo != nullptr) {
        delete localeInfo;
        localeInfo = nullptr;
    }
}

void NumberFormat::InitProperties()
{
    if (!currency.empty()) {
        UErrorCode status = U_ZERO_ERROR;
        numberFormat =
            numberFormat.unit(icu::CurrencyUnit(icu::UnicodeString(currency.c_str()).getBuffer(), status));
        if (currencyDisplay != UNumberUnitWidth::UNUM_UNIT_WIDTH_SHORT) {
            numberFormat = numberFormat.unitWidth(currencyDisplay);
        }
    }
    if (!styleString.empty() && styleString == "percent") {
        numberFormat = numberFormat.unit(icu::NoUnit::percent());
    }
    if (!styleString.empty() && styleString == "unit") {
        for (icu::MeasureUnit curUnit : unitArray) {
            if (strcmp(curUnit.getSubtype(), unit.c_str()) == 0) {
                numberFormat = numberFormat.unit(curUnit);
            }
        }
        numberFormat = numberFormat.unitWidth(unitDisplay);
    }
    if (!useGrouping.empty()) {
        numberFormat.grouping((useGrouping == "true") ?
            UNumberGroupingStrategy::UNUM_GROUPING_AUTO : UNumberGroupingStrategy::UNUM_GROUPING_OFF);
    }
    if (!currencySign.empty() || !signDisplayString.empty()) {
        numberFormat = numberFormat.sign(signDisplay);
    }
    if (!notationString.empty()) {
        numberFormat = numberFormat.notation(notation);
    }
    InitDigitsProperties();
}

void NumberFormat::InitDigitsProperties()
{
    if (!maximumSignificantDigits.empty() || !minimumSignificantDigits.empty()) {
        if (!maximumSignificantDigits.empty()) {
            int32_t maxSignificantDigits = std::stoi(maximumSignificantDigits);
            numberFormat = numberFormat.precision(icu::number::Precision::maxSignificantDigits(maxSignificantDigits));
        }
        if (!minimumSignificantDigits.empty()) {
            int32_t minSignificantDigits = std::stoi(minimumSignificantDigits);
            numberFormat = numberFormat.precision(icu::number::Precision::minSignificantDigits(minSignificantDigits));
        }
    } else {
        if (!minimumIntegerDigits.empty() && std::stoi(minimumIntegerDigits) > 1) {
            numberFormat =
                numberFormat.integerWidth(icu::number::IntegerWidth::zeroFillTo(std::stoi(minimumIntegerDigits)));
        }
        if (!minimumFractionDigits.empty()) {
            numberFormat =
                numberFormat.precision(icu::number::Precision::minFraction(std::stoi(minimumFractionDigits)));
        }
        if (!maximumFractionDigits.empty()) {
            numberFormat =
                numberFormat.precision(icu::number::Precision::maxFraction(std::stoi(maximumFractionDigits)));
        }
    }
}

void NumberFormat::ParseConfigs(std::map<std::string, std::string> &configs)
{
    if (configs.count("signDisplay") > 0) {
        signDisplayString = configs["signDisplay"];
        if (signAutoStyle.count(signDisplayString) > 0) {
            signDisplay = signAutoStyle[signDisplayString];
        }
    }
    if (configs.count("style") > 0) {
        styleString = configs["style"];
    }
    if (styleString == "unit" && configs.count("unit") > 0) {
        unit = configs["unit"];
        if (configs.count("unitDisplay") > 0) {
            unitDisplayString = configs["unitDisplay"];
            unitDisplay = unitStyle[unitDisplayString];
        }
    }
    if (styleString == "currency" && configs.count("currency") > 0) {
        currency = configs["currency"];
        if (configs.count("currencySign") > 0) {
            currencySign = configs["currencySign"];
            if (configs["currencySign"] != "accounting" && !signDisplayString.empty()) {
                signDisplay = signAccountingStyle[signDisplayString];
            }
        }
        if (configs.count("currencyDisplay") > 0 && currencyStyle.count(configs["currencyDisplay"]) > 0) {
            currencyDisplayString = configs["currencyDisplay"];
            currencyDisplay = currencyStyle[currencyDisplayString];
        }
    }
    ParseDigitsConfigs(configs);
}

void NumberFormat::ParseDigitsConfigs(std::map<std::string, std::string> &configs)
{
    if (configs.count("notation") > 0) {
        notationString = configs["notation"];
        if (notationString == "scientific") {
            notation = icu::number::Notation::scientific();
        } else if (notationString == "engineering") {
            notation = icu::number::Notation::engineering();
        }
        if (notationString == "compact" && configs.count("compactDisplay") > 0) {
            compactDisplay = configs["compactDisplay"];
            if (compactDisplay == "long") {
                notation = icu::number::Notation::compactLong();
            } else {
                notation = icu::number::Notation::compactShort();
            }
        }
    }
    if (configs.count("minimumIntegerDigits") > 0) {
        minimumIntegerDigits = configs["minimumIntegerDigits"];
    }
    if (configs.count("minimumFractionDigits") > 0) {
        minimumFractionDigits = configs["minimumFractionDigits"];
    }
    if (configs.count("maximumFractionDigits") > 0) {
        maximumFractionDigits = configs["maximumFractionDigits"];
    }
    if (configs.count("minimumSignificantDigits") > 0) {
        minimumSignificantDigits = configs["minimumSignificantDigits"];
    }
    if (configs.count("maximumSignificantDigits") > 0) {
        maximumSignificantDigits = configs["maximumSignificantDigits"];
    }
    if (configs.count("numberingSystem") > 0) {
        numberingSystem = configs["numberingSystem"];
    }
    if (configs.count("useGrouping") > 0) {
        useGrouping = configs["useGrouping"];
    }
    if (configs.count("localeMatcher") > 0) {
        localeMatcher = configs["localeMatcher"];
    }
}

std::string NumberFormat::Format(double number)
{
    std::string result;
    UErrorCode status = U_ZERO_ERROR;
    numberFormat.formatDouble(number, status).toString(status).toUTF8String(result);
    return result;
}

void NumberFormat::GetResolvedOptions(std::map<std::string, std::string> &map)
{
    map.insert(std::make_pair("locale", localeBaseName));
    if (!styleString.empty()) {
        map.insert(std::make_pair("style", styleString));
    }
    if (!currency.empty()) {
        map.insert(std::make_pair("currency", currency));
    }
    if (!currencySign.empty()) {
        map.insert(std::make_pair("currencySign", currencySign));
    }
    if (!currencyDisplayString.empty()) {
        map.insert(std::make_pair("currencyDisplay", currencyDisplayString));
    }
    if (!signDisplayString.empty()) {
        map.insert(std::make_pair("signDisplay", signDisplayString));
    }
    if (!compactDisplay.empty()) {
        map.insert(std::make_pair("compactDisplay", compactDisplay));
    }
    if (!unitDisplayString.empty()) {
        map.insert(std::make_pair("unitDisplay", unitDisplayString));
    }
    if (!unit.empty()) {
        map.insert(std::make_pair("unit", unit));
    }
    GetDigitsResolvedOptions(map);
}

void NumberFormat::GetDigitsResolvedOptions(std::map<std::string, std::string> &map)
{
    UErrorCode status = U_ZERO_ERROR;
    if (!numberingSystem.empty()) {
        map.insert(std::make_pair("numberingSystem", numberingSystem));
    } else if (!(localeInfo->GetNumberingSystem()).empty()) {
        map.insert(std::make_pair("numberingSystem", localeInfo->GetNumberingSystem()));
    } else {
        auto numSys = std::unique_ptr<icu::NumberingSystem>(icu::NumberingSystem::createInstance(locale, status));
        map.insert(std::make_pair("numberingSystem", numSys->getName()));
    }
    if (!useGrouping.empty()) {
        map.insert(std::make_pair("useGrouping", useGrouping));
    }
    if (!minimumIntegerDigits.empty()) {
        map.insert(std::make_pair("minimumIntegerDigits", minimumIntegerDigits));
    }
    if (!minimumFractionDigits.empty()) {
        map.insert(std::make_pair("minimumFractionDigits", minimumFractionDigits));
    }
    if (!maximumFractionDigits.empty()) {
        map.insert(std::make_pair("maximumFractionDigits", maximumFractionDigits));
    }
    if (!minimumSignificantDigits.empty()) {
        map.insert(std::make_pair("minimumSignificantDigits", minimumSignificantDigits));
    }
    if (!maximumSignificantDigits.empty()) {
        map.insert(std::make_pair("maximumSignificantDigits", maximumSignificantDigits));
    }
    if (!localeMatcher.empty()) {
        map.insert(std::make_pair("localeMatcher", localeMatcher));
    }
    if (!notationString.empty()) {
        map.insert(std::make_pair("notation", notationString));
    }
}

std::string NumberFormat::GetCurrency() const
{
    return currency;
}

std::string NumberFormat::GetCurrencySign() const
{
    return currencySign;
}

std::string NumberFormat::GetStyle() const
{
    return styleString;
}

std::string NumberFormat::GetNumberingSystem() const
{
    return numberingSystem;
}

std::string NumberFormat::GetUseGrouping() const
{
    return useGrouping;
}

std::string NumberFormat::GetMinimumIntegerDigits() const
{
    return minimumIntegerDigits;
}

std::string NumberFormat::GetMinimumFractionDigits() const
{
    return minimumFractionDigits;
}

std::string NumberFormat::GetMaximumFractionDigits() const
{
    return maximumFractionDigits;
}

std::string NumberFormat::GetMinimumSignificantDigits() const
{
    return minimumSignificantDigits;
}

std::string NumberFormat::GetMaximumSignificantDigits() const
{
    return maximumSignificantDigits;
}

std::string NumberFormat::GetLocaleMatcher() const
{
    return localeMatcher;
}

bool NumberFormat::Init()
{
    SetHwIcuDirectory();
    return true;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
