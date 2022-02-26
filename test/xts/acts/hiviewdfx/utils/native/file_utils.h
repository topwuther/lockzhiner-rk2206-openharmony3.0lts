/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef TEST_COMMON_H
#define TEST_COMMON_H
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>

bool CheckInfo(std::vector<std::string> &para, std::string eventinfo);
int ExecCmdWithRet(std::string cmd, std::vector<std::string> &resvec);
std::string ReadFile(std::string filename);
void RedirecthiLog(std::string &hilogredirect, std::string &timeout);
void ExeCmd(std::string cmd);
void CmdRun(std::string cmd, std::string &result);
std::string ExecuteCmd(std::string cmd);
bool CompareString(const std::string& x, const std::string& y);
int GetTxtLine(std::string filename);
std::string ReadFile(std::string filename);
void CleanCmd();
std::string ReadOneLine(std::string m_path, char* rBuf, int n);
void SaveCmdOutput(std::string cmd, std::string saveFile);
std::vector<std::string> getfileinpath(std::string path);
#endif
