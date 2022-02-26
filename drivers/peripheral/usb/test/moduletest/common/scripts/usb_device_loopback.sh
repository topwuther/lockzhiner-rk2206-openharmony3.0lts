#!/bin/bash
# Copyright (c) 2021 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

IFS=$'\n'
read_log_file="/data/acm_read_xts"
max_ts=0
pid=$(ps -ef | grep 'acm_read' | grep -v grep | cut -F 2)
if [ ! "${pid}x" == "x" ];then
    killall acm_read
fi
acm_read &
cat /dev/null > $read_log_file
while true
do
    lines=$(cat $read_log_file)
    cat /dev/null > $read_log_file
    for line in $lines
    do
        ts=$(echo $line | grep 'XTSCHECK' | cut -F 2 | cut -d ',' -f 1)
        if [ `echo "$ts > $max_ts" | bc` -eq 1 ];then
            max_ts=$ts
            data=$(echo $line | grep 'XTSCHECK' | cut -F 4 | cut -d '[|]' -f 2)
            echo "[`date +%s.%N`]" $data
            acm_write $data
        fi
    done
    sleep 0.1
done