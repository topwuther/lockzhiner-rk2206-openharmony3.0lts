#!/usr/bin/env python3

#encoding=utf-8
__author__ = 'addy.ke@rock-chips.com'

import traceback
import os
import sys
import threading
import time
import math
import getopt

chip = 'rk2206'
board = 'lockzhiner-rk2206'

class FlashTool:
    def __init__(self, chip, board):
        self.rootDir = os.getcwd()
        self.outDir = '%s/out/%s/%s' % (self.rootDir, chip, board)
        self.imageDir = '%s/images' % self.outDir
        self.toolDir = '%s/device/rockchip/tools' % self.rootDir
        self.flashTool = 'sudo %s/linux/bin/flash.%s' % (self.toolDir, os.uname().machine)

    def QueryFlashMode(self):
        cmd = '%s LD' % self.flashTool
        fd = os.popen(cmd)
        line = fd.read()
        if line.find("Mode=Loader") >= 0:
            print('loader')
        elif line.find("Mode=Maskrom") >= 0:
            print('maskrom')
        else:
            print("none")

    def FlashLoader(self, chip):
        return '%s db %s/%s_db_loader.bin' % (self.flashTool, self.imageDir, chip)

    def FlashImage(self):
        return '%s wl 0 %s/Firmware.img' % (self.flashTool, self.imageDir)

    def FlashReboot(self):
        return '%s rd' % (self.flashTool)

    def FlashAll(self, chip):
        cmds = []
        cmds.append(tool.FlashLoader(chip))
        cmds.append(tool.FlashImage())
        cmds.append(tool.FlashReboot())
        for cmd in cmds:
            #print(cmd)
            if os.system(cmd) != 0:
                print('\033[0;31mFail to run cmd: %s\033[0m' % cmd)
                break

def help(argv):
    text = 'Usage: ' + argv[0] + 'options [PARAMTER]\n'
    text += '\n'
    text += 'Options:\n'
    text += '  -h, --help                        Print help information\n'
    text += '  -q, --query                       Query device flash mode\n'
    text += '  -a, --all                         Flash all images\n'
    text += '\n'
    text += 'e.g.\n'
    text += '  ' + argv[0] + ' -a\n'
    text +='\n'

    print(text)

try:
    options,args = getopt.getopt(sys.argv[1:], 'hqa', ['help', 'query', 'all'])

except getopt.GetoptError:
    help(sys.argv)
    sys.exit()

tool = FlashTool(chip, board)

if len(options) == 0:
    tool.FlashAll(chip);
    sys.exit()

for option, param in options:
    if option in ('-h', '--help'):
        help(sys.argv)
        sys.exit()
    elif option in ('-q', '--query'):
        tool.QueryFlashMode()
        sys.exit()
    else:
        tool.FlashAll(chip);
