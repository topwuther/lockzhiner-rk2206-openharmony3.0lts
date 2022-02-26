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

#include "matching_skills.h"
#include "event_log_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace EventFwk {
MatchingSkills::MatchingSkills()
{}

MatchingSkills::MatchingSkills(const MatchingSkills &matchingSkills)
{
    entities_ = matchingSkills.entities_;
    events_ = matchingSkills.events_;
    schemes_ = matchingSkills.schemes_;
}

MatchingSkills::~MatchingSkills()
{}

std::string MatchingSkills::GetEntity(int index) const
{
    std::string entity;
    if (0 <= index && index < static_cast<int>(entities_.size())) {
        entity = entities_[index];
    }
    return entity;
}

void MatchingSkills::AddEntity(const std::string &entity)
{
    auto it = std::find(entities_.begin(), entities_.end(), entity);
    if (it == entities_.end()) {
        entities_.emplace_back(entity);
    }
}

bool MatchingSkills::HasEntity(const std::string &entity) const
{
    return std::find(entities_.cbegin(), entities_.cend(), entity) != entities_.cend();
}

void MatchingSkills::RemoveEntity(const std::string &entity)
{
    auto it = std::find(entities_.cbegin(), entities_.cend(), entity);
    if (it != entities_.cend()) {
        entities_.erase(it);
    }
}

int MatchingSkills::CountEntities() const
{
    return entities_.size();
}

void MatchingSkills::AddEvent(const std::string &event)
{
    auto it = std::find(events_.cbegin(), events_.cend(), event);
    if (it == events_.cend()) {
        events_.emplace_back(event);
    }
}

int MatchingSkills::CountEvent() const
{
    return events_.size();
}

std::string MatchingSkills::GetEvent(int index) const
{
    std::string event;
    if (0 <= index && index < static_cast<int>(events_.size())) {
        event = events_[index];
    }
    return event;
}

std::vector<std::string> MatchingSkills::GetEvents() const
{
    return events_;
}

void MatchingSkills::RemoveEvent(const std::string &event)
{
    auto it = std::find(events_.cbegin(), events_.cend(), event);
    if (it != events_.cend()) {
        events_.erase(it);
    }
}

bool MatchingSkills::HasEvent(const std::string &event) const
{
    return std::find(events_.cbegin(), events_.cend(), event) != events_.cend();
}

std::string MatchingSkills::GetScheme(int index) const
{
    std::string schemes;
    if (0 <= index && index < static_cast<int>(schemes_.size())) {
        schemes = schemes_[index];
    }
    return schemes;
}

void MatchingSkills::AddScheme(const std::string &scheme)
{
    auto it = std::find(schemes_.begin(), schemes_.end(), scheme);
    if (it == schemes_.end()) {
        schemes_.emplace_back(scheme);
    }
}

bool MatchingSkills::HasScheme(const std::string &scheme) const
{
    return std::find(schemes_.begin(), schemes_.end(), scheme) != schemes_.end();
}

void MatchingSkills::RemoveScheme(const std::string &scheme)
{
    auto it = std::find(schemes_.cbegin(), schemes_.cend(), scheme);
    if (it != schemes_.cend()) {
        schemes_.erase(it);
    }
}

int MatchingSkills::CountSchemes() const
{
    return schemes_.size();
}

bool MatchingSkills::Marshalling(Parcel &parcel) const
{
    // write entity
    std::vector<std::u16string> actionU16Entity;
    for (std::vector<std::string>::size_type i = 0; i < entities_.size(); i++) {
        actionU16Entity.emplace_back(Str8ToStr16(entities_[i]));
    }
    if (!parcel.WriteString16Vector(actionU16Entity)) {
        EVENT_LOGE("matching skills write entity error");
        return false;
    }

    // write event
    std::vector<std::u16string> actionU16Event;
    for (std::vector<std::string>::size_type i = 0; i < events_.size(); i++) {
        actionU16Event.emplace_back(Str8ToStr16(events_[i]));
    }
    if (!parcel.WriteString16Vector(actionU16Event)) {
        EVENT_LOGE("matching skills write event error");
        return false;
    }

    // write scheme
    std::vector<std::u16string> actionU16Scheme;
    for (std::vector<std::string>::size_type i = 0; i < schemes_.size(); i++) {
        actionU16Scheme.emplace_back(Str8ToStr16(schemes_[i]));
    }
    if (!parcel.WriteString16Vector(actionU16Scheme)) {
        EVENT_LOGE("matching skills write scheme error");
        return false;
    }

    return true;
}

bool MatchingSkills::ReadFromParcel(Parcel &parcel)
{
    // read entity
    std::vector<std::u16string> actionU16Entity;
    if (!parcel.ReadString16Vector(&actionU16Entity)) {
        EVENT_LOGE("matching skills read entity error");
        return false;
    }
    entities_.clear();
    for (std::vector<std::u16string>::size_type i = 0; i < actionU16Entity.size(); i++) {
        entities_.emplace_back(Str16ToStr8(actionU16Entity[i]));
    }

    // read event
    std::vector<std::u16string> actionU16Event;
    if (!parcel.ReadString16Vector(&actionU16Event)) {
        EVENT_LOGE("matching skills read event error");
        return false;
    }
    events_.clear();
    for (std::vector<std::u16string>::size_type i = 0; i < actionU16Event.size(); i++) {
        events_.emplace_back(Str16ToStr8(actionU16Event[i]));
    }

    // read scheme
    std::vector<std::u16string> actionU16Scheme;
    if (!parcel.ReadString16Vector(&actionU16Scheme)) {
        EVENT_LOGE("matching skills read scheme error");
        return false;
    }
    schemes_.clear();
    for (std::vector<std::u16string>::size_type i = 0; i < actionU16Scheme.size(); i++) {
        schemes_.emplace_back(Str16ToStr8(actionU16Scheme[i]));
    }
    return true;
}

MatchingSkills *MatchingSkills::Unmarshalling(Parcel &parcel)
{
    MatchingSkills *matchingSkills = new MatchingSkills();

    if (matchingSkills && !matchingSkills->ReadFromParcel(parcel)) {
        delete matchingSkills;
        matchingSkills = nullptr;
    }

    return matchingSkills;
}

bool MatchingSkills::MatchEvent(const std::string &event) const
{
    if (event == std::string()) {
        EVENT_LOGD("event is null");
        return false;
    }

    return HasEvent(event);
}

bool MatchingSkills::MatchEntity(const std::vector<std::string> &entities) const
{
    if (entities.size() == 0) {
        return true;
    }

    for (auto vec : entities) {
        auto it = std::find(entities_.cbegin(), entities_.cend(), vec);
        if (it == entities_.cend()) {
            return false;
        }
    }

    return true;
}

bool MatchingSkills::MatchScheme(const std::string &scheme) const
{
    if (schemes_.size()) {
        return HasScheme(scheme);
    }

    if (scheme == std::string()) {
        EVENT_LOGD("scheme is null");
        return true;
    }

    return false;
}

bool MatchingSkills::Match(const Want &want) const
{
    return MatchEvent(want.GetAction()) && MatchEntity(want.GetEntities()) && MatchScheme(want.GetScheme());
}
}  // namespace EventFwk
}  // namespace OHOS