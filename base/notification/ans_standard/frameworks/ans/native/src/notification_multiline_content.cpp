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

#include <algorithm>
#include "notification_multiline_content.h"
#include "ans_log_wrapper.h"

namespace OHOS {
namespace Notification {
const std::vector<std::string>::size_type NotificationMultiLineContent::MAX_LINES {7};

void NotificationMultiLineContent::SetExpandedTitle(const std::string &exTitle)
{
    expandedTitle_ = exTitle;
}

std::string NotificationMultiLineContent::GetExpandedTitle() const
{
    return expandedTitle_;
}

void NotificationMultiLineContent::SetBriefText(const std::string &briefText)
{
    briefText_ = briefText;
}

std::string NotificationMultiLineContent::GetBriefText() const
{
    return briefText_;
}

void NotificationMultiLineContent::AddSingleLine(const std::string &oneLine)
{
    if (allLines_.size() >= NotificationMultiLineContent::MAX_LINES) {
        ANS_LOGW("already added seven lines");
        return;
    }

    allLines_.emplace_back(oneLine);
}

std::vector<std::string> NotificationMultiLineContent::GetAllLines() const
{
    return allLines_;
}

std::string NotificationMultiLineContent::Dump()
{
    std::string lines {};
    std::for_each(
        allLines_.begin(), allLines_.end(), [&lines](const std::string &line) { lines += " " + line + ","; });
    lines.pop_back();

    return "NotificationMultiLineContent[ " + NotificationBasicContent::Dump() + ", briefText = " + briefText_ +
           ", expandedTitle = " + expandedTitle_ + ", allLines = [" + lines + " ] ]";
}

bool NotificationMultiLineContent::Marshalling(Parcel &parcel) const
{
    if (!NotificationBasicContent::Marshalling(parcel)) {
        ANS_LOGE("Failed to write basic");
        return false;
    }

    if (!parcel.WriteString(expandedTitle_)) {
        ANS_LOGE("Failed to write expanded title");
        return false;
    }

    if (!parcel.WriteString(briefText_)) {
        ANS_LOGE("Failed to write brief text");
        return false;
    }

    if (!parcel.WriteStringVector(allLines_)) {
        ANS_LOGE("Failed to write all lines");
        return false;
    }

    return true;
}

NotificationMultiLineContent *NotificationMultiLineContent::Unmarshalling(Parcel &parcel)
{
    auto pContent = new NotificationMultiLineContent();
    if ((nullptr != pContent) && !pContent->ReadFromParcel(parcel)) {
        delete pContent;
        pContent = nullptr;
    }

    return pContent;
}

bool NotificationMultiLineContent::ReadFromParcel(Parcel &parcel)
{
    if (!NotificationBasicContent::ReadFromParcel(parcel)) {
        ANS_LOGE("Failed to read basic");
        return false;
    }

    if (!parcel.ReadString(expandedTitle_)) {
        ANS_LOGE("Failed to read expanded title");
        return false;
    }

    if (!parcel.ReadString(briefText_)) {
        ANS_LOGE("Failed to read brief text");
        return false;
    }

    if (!parcel.ReadStringVector(&allLines_)) {
        ANS_LOGE("Failed to read all lines");
        return false;
    }

    return true;
}
}  // namespace Notification
}  // namespace OHOS
