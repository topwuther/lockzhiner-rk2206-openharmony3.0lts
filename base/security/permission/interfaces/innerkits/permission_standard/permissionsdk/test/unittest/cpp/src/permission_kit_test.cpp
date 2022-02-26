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

#include "permission_kit_test.h"

#include "test_const.h"
#include "permission_kit.h"

using namespace testing::ext;
using namespace OHOS::Security::Permission;

void PermissionKitTest::SetUpTestCase()
{}

void PermissionKitTest::TearDownTestCase()
{
    PermissionKit::RemoveDefPermissions(TEST_BUNDLE_NAME);
    PermissionKit::RemoveUserGrantedReqPermissions(TEST_BUNDLE_NAME, TEST_USER_ID);
    PermissionKit::RemoveSystemGrantedReqPermissions(TEST_BUNDLE_NAME);
}

void PermissionKitTest::SetUp()
{
    vector<PermissionDef> permDefList;
    PermissionDef permissionDefAlpha = {
        .permissionName = TEST_PERMISSION_NAME_ALPHA,
        .bundleName = TEST_BUNDLE_NAME,
        .grantMode = GrantMode::USER_GRANT,
        .availableScope = AVAILABLE_SCOPE_ALL,
        .label = TEST_LABEL,
        .labelId = TEST_LABEL_ID,
        .description = TEST_DESCRIPTION,
        .descriptionId = TEST_DESCRIPTION_ID
    };

    PermissionDef permissionDefBeta = {
        .permissionName = TEST_PERMISSION_NAME_BETA,
        .bundleName = TEST_BUNDLE_NAME,
        .grantMode = GrantMode::SYSTEM_GRANT,
        .availableScope = AVAILABLE_SCOPE_ALL,
        .label = TEST_LABEL,
        .labelId = TEST_LABEL_ID,
        .description = TEST_DESCRIPTION,
        .descriptionId = TEST_DESCRIPTION_ID
    };

    permDefList.emplace_back(permissionDefAlpha);
    permDefList.emplace_back(permissionDefBeta);
    PermissionKit::AddDefPermissions(permDefList);
}

void PermissionKitTest::TearDown()
{}

/**
 * @tc.name: AddDefPermissions001
 * @tc.desc: Get permission definition info successfully after AddDefPermissions function has been invoked
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionKitTest, AddDefPermissions001, TestSize.Level1)
{
    PermissionDef permissionDefResultAlpha;
    int ret = PermissionKit::GetDefPermission(TEST_PERMISSION_NAME_ALPHA, permissionDefResultAlpha);
    ASSERT_EQ(RET_SUCCESS, ret);

    PermissionDef permissionDefResultBeta;
    ret = PermissionKit::GetDefPermission(TEST_PERMISSION_NAME_BETA, permissionDefResultBeta);
    ASSERT_EQ(RET_SUCCESS, ret);
}

/**
 * @tc.name: RemoveDefPermissions001
 * @tc.desc: Cannot get permission definition info after RemoveDefPermissions has been invoked
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionKitTest, RemoveDefPermissions001, TestSize.Level1)
{
    PermissionDef permissionDefResultAlpha;
    int ret = PermissionKit::GetDefPermission(TEST_PERMISSION_NAME_ALPHA, permissionDefResultAlpha);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::RemoveDefPermissions(TEST_BUNDLE_NAME);
    ASSERT_EQ(RET_SUCCESS, ret);

    PermissionDef result;
    ret = PermissionKit::GetDefPermission(TEST_PERMISSION_NAME_ALPHA, result);
    ASSERT_EQ(RET_FAILED, ret);
}

/**
 * @tc.name: VerifyPermission001
 * @tc.desc: Verify user granted permission
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionKitTest, VerifyPermission001, TestSize.Level1)
{
    vector<string> permList;
    permList.push_back(TEST_PERMISSION_NAME_ALPHA);

    int ret = PermissionKit::AddUserGrantedReqPermissions(TEST_BUNDLE_NAME, permList, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::GrantUserGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_GRANTED, ret);

    ret = PermissionKit::RevokeUserGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::RemoveUserGrantedReqPermissions(TEST_BUNDLE_NAME, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);
}

/**
 * @tc.name: VerifyPermission002
 * @tc.desc: Verify system granted permission
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionKitTest, VerifyPermission002, TestSize.Level1)
{
    vector<string> permList;
    permList.push_back(TEST_PERMISSION_NAME_BETA);

    int ret = PermissionKit::AddSystemGrantedReqPermissions(TEST_BUNDLE_NAME, permList);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::GrantSystemGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_GRANTED, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_SUB_USER_ID);
    ASSERT_EQ(PERMISSION_GRANTED, ret);

    ret = PermissionKit::RevokeSystemGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_SUB_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::RemoveSystemGrantedReqPermissions(TEST_BUNDLE_NAME);
    ASSERT_EQ(RET_SUCCESS, ret);
}

/**
 * @tc.name: VerifyPermission003
 * @tc.desc: Verify permission that has not been defined.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionKitTest, VerifyPermission003, TestSize.Level1)
{
    int ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_GAMMA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);
}

/**
 * @tc.name: VerifyPermissionErrorUserGrant001
 * @tc.desc: Verify permission error that user granted but request system granted.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionKitTest, VerifyPermissionErrorUserGrant001, TestSize.Level1)
{
    vector<string> permList;
    permList.push_back(TEST_PERMISSION_NAME_ALPHA);

    int ret = PermissionKit::AddSystemGrantedReqPermissions(TEST_BUNDLE_NAME, permList);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::GrantUserGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::RevokeUserGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::RemoveUserGrantedReqPermissions(TEST_BUNDLE_NAME, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);
}

/**
 * @tc.name: VerifyPermissionErrorUserGrant002
 * @tc.desc: Verify permission error that user granted but grant system granted.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionKitTest, VerifyPermissionErrorUserGrant002, TestSize.Level1)
{
    vector<string> permList;
    permList.push_back(TEST_PERMISSION_NAME_ALPHA);

    int ret = PermissionKit::AddUserGrantedReqPermissions(TEST_BUNDLE_NAME, permList, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::GrantSystemGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::RevokeUserGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::RemoveUserGrantedReqPermissions(TEST_BUNDLE_NAME, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);
}

/**
 * @tc.name: VerifyPermissionErrorSystemGrant001
 * @tc.desc: Verify permission error that system granted but request user granted.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionKitTest, VerifyPermissionErrorSystemGrant001, TestSize.Level1)
{
    vector<string> permList;
    permList.push_back(TEST_PERMISSION_NAME_BETA);

    int ret = PermissionKit::AddUserGrantedReqPermissions(TEST_BUNDLE_NAME, permList, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::GrantSystemGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::RevokeSystemGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::RemoveUserGrantedReqPermissions(TEST_BUNDLE_NAME, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);
}

/**
 * @tc.name: VerifyPermissionErrorSystemGrant002
 * @tc.desc: Verify permission error that system granted but grant user granted.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionKitTest, VerifyPermissionErrorSystemGrant002, TestSize.Level1)
{
    vector<string> permList;
    permList.push_back(TEST_PERMISSION_NAME_BETA);

    int ret = PermissionKit::AddSystemGrantedReqPermissions(TEST_BUNDLE_NAME, permList);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::GrantUserGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_SUB_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::RevokeSystemGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA);
    ASSERT_EQ(RET_SUCCESS, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::VerifyPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_SUB_USER_ID);
    ASSERT_EQ(PERMISSION_NOT_GRANTED, ret);

    ret = PermissionKit::RemoveUserGrantedReqPermissions(TEST_BUNDLE_NAME, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);
}

/**
 * @tc.name: CanRequestPermission001
 * @tc.desc: For user granted permission and permission is granted, can request permission
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionKitTest, CanRequestPermission001, TestSize.Level1)
{
    vector<string> permList;
    permList.push_back(TEST_PERMISSION_NAME_ALPHA);

    int ret = PermissionKit::AddUserGrantedReqPermissions(TEST_BUNDLE_NAME, permList, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);

    bool isCanRequest = PermissionKit::CanRequestPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_TRUE(isCanRequest);

    ret = PermissionKit::GrantUserGrantedPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_EQ(RET_SUCCESS, ret);

    isCanRequest = PermissionKit::CanRequestPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_FALSE(isCanRequest);

    ret = PermissionKit::RemoveDefPermissions(TEST_BUNDLE_NAME);
    ASSERT_EQ(RET_SUCCESS, ret);

    isCanRequest = PermissionKit::CanRequestPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_ALPHA, TEST_USER_ID);
    ASSERT_FALSE(isCanRequest);
}

/**
 * @tc.name: CanRequestPermission001
 * @tc.desc: For non user granted permission, can not request permission
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(PermissionKitTest, CanRequestPermission002, TestSize.Level1)
{
    vector<string> permList;
    permList.push_back(TEST_PERMISSION_NAME_BETA);

    int ret = PermissionKit::AddSystemGrantedReqPermissions(TEST_BUNDLE_NAME, permList);
    ASSERT_EQ(RET_SUCCESS, ret);

    bool isCanRequest = PermissionKit::CanRequestPermission(TEST_BUNDLE_NAME, TEST_PERMISSION_NAME_BETA, TEST_USER_ID);
    ASSERT_FALSE(isCanRequest);

    isCanRequest = PermissionKit::CanRequestPermission("", "", TEST_INVALID_USER_ID);
    ASSERT_FALSE(isCanRequest);
}