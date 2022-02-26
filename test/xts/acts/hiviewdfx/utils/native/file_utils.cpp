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
#include "file_utils.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <dirent.h>
#include <condition_variable>
#include <vector>
#include "include/securec.h"
#include "string_util.h"

using namespace OHOS::HiviewDFX;
int ExecCmdWithRet(std::string cmd, std::vector<std::string> &resvec)
{
    if (cmd.size() == 0) {
        return 0;
    }

    std::cout<< "cmd is " + cmd <<std::endl;
    if ((cmd.find("hilog") == std::string::npos) && (cmd.find("hidumper") == std::string::npos)
        && (cmd.find("ps") == std::string::npos) && (cmd.find("rm") == std::string::npos)) {
        std::cout<<"unsupport cmd!" + cmd <<std::endl;
        return 0;
    }
    resvec.clear();
    FILE *pp = popen(cmd.c_str(), "r");
    if (pp == nullptr) {
        return -1;
    }
    char tmp[1024];
    while (fgets(tmp, sizeof(tmp), pp) != nullptr) {
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0';
        }
        resvec.push_back(tmp);
    }
    pclose(pp);
    return resvec.size();
}

void ExeCmd(std::string cmd)
{
    std::vector<std::string> cmdret;
    ExecCmdWithRet(cmd, cmdret);
}

void CmdRun(std::string cmd, std::string &result)
{
    std::vector<std::string> cmdret;
    int resultlen;
    int i = 0;
    std::string rst;
    resultlen = ExecCmdWithRet(cmd, cmdret);
    while (i < resultlen) {
        rst = rst + cmdret[i];
        i = i + 1;
    }
    result = rst;
}

void CleanCmd()
{
    std::string cmdResult;
    std::string cleanCmd = "hilog -r";
    CmdRun(cleanCmd, cmdResult);
    std::cout << cmdResult;
}

std::string ExecuteCmd(std::string cmd)
{
    std::vector<std::string> cmdret;
    int resultlen;
    int i = 0;
    std::string rst;
    resultlen = ExecCmdWithRet(cmd, cmdret);
    while (i < resultlen) {
        rst = rst + cmdret[i] + "\n";
        i = i + 1;
    }
    return rst;
}
void SaveCmdOutput(std::string cmd, std::string saveFile)
{
    std::fstream fstr(saveFile, std::ios::out);
    std::string cmdRet = ExecuteCmd(cmd);
    fstr << cmdRet;
    fstr.close();
}
void RedirecthiLog(std::string &hilogredirect, std::string &timeout)
{
    unsigned long i;
    std::vector<std::string> cmdret;
    unsigned long cmdretlen;
    std::string cmd = "rm " + hilogredirect;
    cmdretlen = ExecCmdWithRet(cmd, cmdret);
    for (i = 0; i < cmdretlen; i++) {
        std::cout<<cmdret[i].c_str()<<std::endl;
    }
    cmd = "timeout " + timeout + " hilog >" + hilogredirect;
    std::cout<<cmd<<std::endl;
    cmdretlen = ExecCmdWithRet(cmd, cmdret);
    for (i = 0; i < cmdretlen; i++) {
        std::cout<<cmdret[i].c_str()<<std::endl;
    }
}

bool CheckInfo(std::vector<std::string> &para, std::string info)
{
    if (info.empty()) {
        return false;
    }
    std::vector<std::string> splitStr;
    StringUtil::SplitStr(info, "\n", splitStr, false, false);
    unsigned long matchcnt;
    bool result = false;
    std::string eventinfoline;
    std::cout<<info<<std::endl;
    matchcnt = 0;
    for (std::vector<std::string>::iterator iter = splitStr.begin(); iter != splitStr.end(); ++iter) {
        eventinfoline = std::string (iter->c_str());
        for (unsigned long i = 0; i < para.size(); i++) {
            std::cout<<para[i]<<std::endl;
            if (int(eventinfoline.find(para[i])) >= 0) {
                matchcnt++;
            }
        }
        std::cout<<"Expect1:"<<std::endl;
        if (matchcnt == para.size()) {
            std::cout<<"Expect2:"<<std::endl;
            result = true;
            break;
        }
    }
    return result;
}

bool CompareString(const std::string& x, const std::string& y)
{
    int len = x.length() - 1;
    while (x[len] == y[len] && len >= 0) {
        len--;
    }
    if (len >= 0 && x[len] > y[len]) {
        return false;
    }
    return true;
}

int GetTxtLine(std::string filename)
{
    FILE *fd = fopen(filename.c_str(), "r");
    int count = 0;
    if (fd != nullptr) {
        while (!feof(fd)) {
            if (fgetc(fd) == '\n') {
                count++;
            }
        }
    }
    std::cout << count << std::endl;
    if (fd != nullptr) {
        fclose(fd);
    }
    return count;
}

std::string ReadFile(std::string filename)
{
    std::ifstream ifile(filename);
    std::ostringstream buf;
    char ch;
    if (ifile.fail()) {
        std::cout<<"open file fail!"<<std::endl;
        return "";
    }
    while (buf && ifile.get(ch)) {
        buf.put(ch);
    }
    ifile.close();
    return buf.str();
}

std::vector<std::string> getfileinpath(std::string path)
{
    std::vector<std::string> filelist;
    DIR *dir;
    struct dirent *pdirent;
    dir = opendir(path.c_str());
    if (dir != nullptr) {
        while ((pdirent = readdir(dir)) != nullptr) {
            if (strncmp(pdirent->d_name, ".", strlen(pdirent->d_name)) == 0 ||
                strncmp(pdirent->d_name, "..", strlen(pdirent->d_name)) == 0) {
                continue;
            } else if (pdirent->d_type == 4) {
                continue;
            } else {
                filelist.push_back(pdirent->d_name);
            }
        }
        closedir(dir);
    }
    return filelist;
}
#endif
