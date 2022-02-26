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

#include "permission_definition_manager_test.h"

#include <string>

#include "permission_definition_manager.h"
#include "permission.h"
#include "test_const.h"

using namespace testing::ext;
using namespace OHOS::Security::Permission;

void PermissionDefinitionManagerTest::SetUpTestCase()
{}

void PermissionDefinitionManagerTest::TearDownTestCase()
{}

void PermissionDefinitionManagerTest::SetUp()
{}

void PermissionDefinitionManagerTest::TearDown()
{
    PermissionDefinitionManager::GetInstance().RemoveDefPermissions(TEST_BUNDLE_NAME);
}

HWTEST_F(PermissionDefinitionManagerTest, AddDefPermissions001, TestSize.Level1)
{
    vector<PermissionDefParcel> defs;
    PermissionDefParcel permissionDefParcel;
    permissionDefParcel.permissionDef.permissionName = TEST_PERMISSION_NAME_ALPHA;
    permissionDefParcel.permissionDef.bundleName = TEST_BUNDLE_NAME;
    permissionDefParcel.permissionDef.grantMode = GrantMode::SYSTEM_GRANT;
    permissionDefParcel.permissionDef.availableScope = AvailableScope::AVAILABLE_SCOPE_ALL;
    permissionDefParcel.permissionDef.label = TEST_LABEL;
    permissionDefParcel.permissionDef.labelId = TEST_LABEL_ID;
    permissionDefParcel.permissionDef.description = TEST_DESCRIPTION;
    permissionDefParcel.permissionDef.descriptionId = TEST_DESCRIPTION_ID;
    defs.emplace_back(permissionDefParcel);
    PermissionDefinitionManager::GetInstance().AddDefPermissions(defs);

    PermissionDefParcel result;
    PermissionDefinitionManager::GetInstance().GetDefPermission(TEST_PERMISSION_NAME_ALPHA, result);
    ASSERT_EQ(TEST_PERMISSION_NAME_ALPHA, result.permissionDef.permissionName);
    ASSERT_EQ(TEST_BUNDLE_NAME, result.permissionDef.bundleName);
    ASSERT_EQ(GrantMode::SYSTEM_GRANT, result.permissionDef.grantMode);
    ASSERT_EQ(AvailableScope::AVAILABLE_SCOPE_ALL, result.permissionDef.availableScope);
    ASSERT_EQ(TEST_LABEL, result.permissionDef.label);
    ASSERT_EQ(TEST_LABEL_ID, result.permissionDef.labelId);
    ASSERT_EQ(TEST_DESCRIPTION, result.permissionDef.description);
    ASSERT_EQ(TEST_DESCRIPTION_ID, result.permissionDef.descriptionId);
}

HWTEST_F(PermissionDefinitionManagerTest, RemoveDefPermissions001, TestSize.Level1)
{
    vector<PermissionDefParcel> defs;
    PermissionDefParcel permissionDefParcel;
    permissionDefParcel.permissionDef.permissionName = TEST_PERMISSION_NAME_ALPHA;
    permissionDefParcel.permissionDef.bundleName = TEST_BUNDLE_NAME;
    permissionDefParcel.permissionDef.grantMode = GrantMode::SYSTEM_GRANT;
    permissionDefParcel.permissionDef.availableScope = AVAILABLE_SCOPE_SIGNATURE;
    defs.emplace_back(permissionDefParcel);
    PermissionDefinitionManager::GetInstance().AddDefPermissions(defs);

    PermissionDefParcel result;
    int ret = PermissionDefinitionManager::GetInstance().GetDefPermission(TEST_PERMISSION_NAME_ALPHA, result);
    ASSERT_EQ(RET_SUCCESS, ret);

    PermissionDefinitionManager::GetInstance().RemoveDefPermissions(TEST_BUNDLE_NAME);

    PermissionDefParcel defParcel;
    ret = PermissionDefinitionManager::GetInstance().GetDefPermission(TEST_PERMISSION_NAME_ALPHA, defParcel);
    ASSERT_EQ(RET_FAILED, ret);
}
