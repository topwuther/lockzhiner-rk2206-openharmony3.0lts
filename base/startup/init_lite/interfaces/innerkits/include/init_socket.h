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

#ifndef INIT_SOCKET_API_H
#define INIT_SOCKET_API_H

#define OHOS_SOCKET_DIR    "/dev/unix/socket"
#define OHOS_SOCKET_ENV_PREFIX    "OHOS_SOCKET_"
// parameter is service name
int GetControlSocket(const char *name);

#endif