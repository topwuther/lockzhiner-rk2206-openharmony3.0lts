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

#include "date_time_data.h"
#include <cstring>
#include "str_util.h"

using namespace OHOS::I18N;

using namespace std;

DateTimeData::DateTimeData(const char *amPmMarkers, const char *sepAndHour, const int size)
{
    if (amPmMarkers != nullptr) {
        size_t len = strlen(const_cast<char*>(amPmMarkers));
        if (len > 0) {
            this->amPmMarkers = NewArrayAndCopy(amPmMarkers, len);
        }
    }
    // size must >= 2, The first 2 element of sepAndHour need to be extracted, the first element
    // is the time separator and the second is the default hour.
    if (sepAndHour && size >= 2) {
        timeSeparator = sepAndHour[0];
        defaultHour = sepAndHour[1];
    }
}

string DateTimeData::GetMonthName(int32_t index, DateTimeDataType type)
{
    if ((index < 0) || (index >= MONTH_SIZE)) {
        return "";
    }
    switch (type) {
        case DateTimeDataType::FORMAT_ABBR: {
            return Parse(formatAbbreviatedMonthNames, index);
        }
        case DateTimeDataType::FORMAT_WIDE: {
            return Parse(formatWideMonthNames, index);
        }
        case DateTimeDataType::STANDALONE_ABBR: {
            return Parse(standaloneAbbreviatedMonthNames, index);
        }
        default: {
            return Parse(standaloneWideMonthNames, index);
        }
    }
}

string DateTimeData::GetDayName(int32_t index, DateTimeDataType type)
{
    if ((index < 0) || (index >= DAY_SIZE)) {
        return "";
    }
    switch (type) {
        case DateTimeDataType::FORMAT_ABBR: {
            return Parse(formatAbbreviatedDayNames, index);
        }
        case DateTimeDataType::FORMAT_WIDE: {
            return Parse(formatWideDayNames, index);
        }
        case DateTimeDataType::STANDALONE_ABBR: {
            return Parse(standaloneAbbreviatedDayNames, index);
        }
        default: {
            return Parse(standaloneWideDayNames, index);
        }
    }
}

string DateTimeData::GetAmPmMarker(int32_t index, DateTimeDataType type)
{
    if ((index < 0) || (index >= AM_SIZE)) {
        return "";
    }
    return (amPmMarkers != "") ? Parse(amPmMarkers, index) : "";
}

char DateTimeData::GetTimeSeparator(void) const
{
    return timeSeparator;
}

char DateTimeData::GetDefaultHour(void) const
{
    return defaultHour;
}

DateTimeData::~DateTimeData()
{
    I18nFree(formatAbbreviatedMonthNames);
    I18nFree(formatWideMonthNames);
    I18nFree(standaloneAbbreviatedMonthNames);
    I18nFree(standaloneWideMonthNames);
    I18nFree(formatAbbreviatedDayNames);
    I18nFree(formatWideDayNames);
    I18nFree(standaloneAbbreviatedDayNames);
    I18nFree(standaloneWideDayNames);
    I18nFree(timePatterns);
    I18nFree(datePatterns);
    I18nFree(amPmMarkers);
    I18nFree(hourMinuteSecondPatterns);
    I18nFree(fullMediumShortPatterns);
    I18nFree(elapsedPatterns);
}

void DateTimeData::SetMonthNamesData(const char *formatAbbreviatedMonthNames, const char *formatWideMonthNames,
    const char *standaloneAbbreviatedMonthNames, const char *standaloneWideMonthNames)
{
    if ((formatAbbreviatedMonthNames == nullptr) || (formatWideMonthNames == nullptr) ||
        (standaloneAbbreviatedMonthNames == nullptr) || (standaloneWideMonthNames == nullptr)) {
        return;
    }
    this->formatAbbreviatedMonthNames = NewArrayAndCopy(formatAbbreviatedMonthNames,
        strlen(formatAbbreviatedMonthNames));
    this->formatWideMonthNames = NewArrayAndCopy(formatWideMonthNames, strlen(formatWideMonthNames));
    this->standaloneAbbreviatedMonthNames = NewArrayAndCopy(standaloneAbbreviatedMonthNames,
        strlen(standaloneAbbreviatedMonthNames));
    this->standaloneWideMonthNames = NewArrayAndCopy(standaloneWideMonthNames, strlen(standaloneWideMonthNames));
}

void DateTimeData::SetDayNamesData(const char *formatAbbreviatedDayNames, const char *formatWideDayNames,
    const char *standaloneAbbreviatedDayNames, const char *standaloneWideDayNames)
{
    if ((formatAbbreviatedDayNames == nullptr) || (formatWideDayNames == nullptr) ||
        (standaloneAbbreviatedDayNames == nullptr) || (standaloneWideDayNames == nullptr)) {
        return;
    }
    this->formatAbbreviatedDayNames = NewArrayAndCopy(formatAbbreviatedDayNames,
        strlen(formatAbbreviatedDayNames));
    this->formatWideDayNames = NewArrayAndCopy(formatWideDayNames, strlen(formatWideDayNames));
    this->standaloneAbbreviatedDayNames = NewArrayAndCopy(standaloneAbbreviatedDayNames,
        strlen(standaloneAbbreviatedDayNames));
    this->standaloneWideDayNames = NewArrayAndCopy(standaloneWideDayNames, strlen(standaloneWideDayNames));
}

void DateTimeData::SetPatternsData(const char *datePatterns, const char *timePatterns,
    const char *hourMinuteSecondPatterns, const char *fullMediumShortPatterns, const char *elapsedPatterns)
{
    if ((datePatterns == nullptr) || (timePatterns == nullptr) ||
        (hourMinuteSecondPatterns == nullptr) || (fullMediumShortPatterns == nullptr) ||
        (elapsedPatterns == nullptr)) {
        return;
    }
    size_t timeLength = strlen(timePatterns);
    size_t dateLength = strlen(datePatterns);
    size_t hourLength = strlen(hourMinuteSecondPatterns);
    size_t fullLength = strlen(fullMediumShortPatterns);
    size_t elapsedLength = strlen(elapsedPatterns);
    if ((timeLength == 0) || (dateLength == 0) || (hourLength == 0) || (fullLength == 0)) {
        return;
    }
    I18nFree(this->timePatterns);
    this->timePatterns = NewArrayAndCopy(timePatterns, timeLength);
    I18nFree(this->datePatterns);
    this->datePatterns = NewArrayAndCopy(datePatterns, dateLength);
    I18nFree(this->hourMinuteSecondPatterns);
    this->hourMinuteSecondPatterns = NewArrayAndCopy(hourMinuteSecondPatterns, hourLength);
    I18nFree(this->fullMediumShortPatterns);
    this->fullMediumShortPatterns = NewArrayAndCopy(fullMediumShortPatterns, fullLength);
    I18nFree(this->elapsedPatterns);
    this->elapsedPatterns = NewArrayAndCopy(elapsedPatterns, elapsedLength);
}
