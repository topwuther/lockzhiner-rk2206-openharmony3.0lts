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

#ifndef FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_MATCHING_SKILLS_H
#define FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_MATCHING_SKILLS_H

#include <string>
#include <vector>

#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace EventFwk {
using Want = OHOS::AAFwk::Want;

class MatchingSkills : public Parcelable {
public:
    /**
     * A constructor used to create an empty MatchingSkills instance.
     */
    MatchingSkills();

    /**
     * A constructor used to create a MatchingSkills instance by using the parameters of an existing MatchingSkills
     * object.
     */
    MatchingSkills(const MatchingSkills &matchingSkills);

    /**
     * A deconstructor used to deconstruct
     */
    ~MatchingSkills();

    /**
     * Obtain an entity.
     *
     * @return entity
     */
    std::string GetEntity(int index) const;

    /**
     * Add an event to this MatchingSkills object.
     *
     * @param entity the entity
     */
    void AddEntity(const std::string &entity);

    /**
     * Has entity.
     *
     * @param event  the entity in MatchingSkills
     * @return whether the entity in MatchingSkills or not
     */
    bool HasEntity(const std::string &entity) const;

    /**
     * Remove entity.
     *
     * @param event  the entity in MatchingSkills
     * @return remove entity success or not
     */
    void RemoveEntity(const std::string &entity);

    /**
     * Gets entity count.
     *
     * @return entity count
     */
    int CountEntities() const;

    /**
     * Adds an event to this MatchingSkills object.
     *
     * @param event the event
     */
    void AddEvent(const std::string &event);

    /**
     * Get event count.
     *
     * @return the event count in MatchingSkills
     */
    int CountEvent() const;

    /**
     * Get event.
     *
     * @return the event in MatchingSkills
     */
    std::string GetEvent(int index) const;

    /**
     * Get events.
     *
     * @return the events in MatchingSkills
     */
    std::vector<std::string> GetEvents() const;

    /**
     * Remove events.
     *
     * @return event the event in MatchingSkills
     */
    void RemoveEvent(const std::string &event);

    /**
     * Has event.
     *
     * @param event the event in MatchingSkills
     * @return whether the event in MatchingSkills or not
     */
    bool HasEvent(const std::string &event) const;

    /**
     * obtain an Scheme.
     *
     * @return Scheme
     */
    std::string GetScheme(int index) const;

    /**
     * Add scheme to this MatchingSkills object.
     *
     * @param scheme the scheme
     */
    void AddScheme(const std::string &scheme);

    /**
     * Has scheme.
     *
     * @param event  the scheme in MatchingSkills
     * @return whether the scheme in MatchingSkills or not
     */
    bool HasScheme(const std::string &scheme) const;

    /**
     * Remove entity.
     *
     * @param event  the entity in MatchingSkills
     * @return remove entity success or not
     */
    void RemoveScheme(const std::string &scheme);

    /**
     * Get scheme count.
     *
     * @return scheme count
     */
    int CountSchemes() const;

    /**
     * Match want.
     *
     * @param want the want in MatchingSkills
     * @return match the want result
     */
    bool Match(const Want &want) const;

    /**
     * Marshal this MatchingSkills object into a Parcel.
     *
     * @param parcel  parcel
     * @return marshalling result
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * Unmarshal this MatchingSkills object from a Parcel.
     *
     * @param parcel  parcel
     * @return MatchingSkills
     */
    static MatchingSkills *Unmarshalling(Parcel &parcel);

private:
    /**
     * Read MatchingSkills object from a Parcel.
     *
     * @param parcel the parcel
     * @return read from parcel success or fail
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * Match event.
     *
     * @param parcel the event in want
     * @return match event success or fail
     */
    bool MatchEvent(const std::string &event) const;

    /**
     * Match entity.
     *
     * @param parcel the entity in want
     * @return match entity success or fail
     */
    bool MatchEntity(const std::vector<std::string> &entities) const;

    /**
     * Match scheme.
     *
     * @param parcel the scheme in want
     * @return match scheme success or fail
     */
    bool MatchScheme(const std::string &scheme) const;

private:
    std::vector<std::string> entities_;
    std::vector<std::string> events_;
    std::vector<std::string> schemes_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_MATCHING_SKILLS_H