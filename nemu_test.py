#!/usr/bin/env python3

import os
import subprocess
import sys
import pathlib

NEMU_HOME = None

def check_env():
    nemu_home = os.environ.get("NEMU_HOME")
    if not nemu_home:
        return False

    if not os.path.isdir(nemu_home):
        return False
    global NEMU_HOME
    NEMU_HOME = pathlib.Path(nemu_home)

    return True

def run_nemu(cmd_list):
    try:
        process = subprocess.Popen(
            ["make", "run"],
            cwd=NEMU_HOME,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        for cmd in cmd_list:
            process.stdin.write(cmd)
            process.stdin.write("\n")
            process.stdin.flush()
        output,_  = process.communicate("q", timeout=10)
        output = output.split("(nemu)")
        assert len(cmd_list) == len(output) - 1, "Mismatch between commands and output."
        return process.returncode, output[1:] # 第一段内容是系统信息, 不是命令产生的
    except Exception:
        return None, None

def check_exit_gracefully():
    if not NEMU_HOME:
        return False

    ret,_  = run_nemu(["q"])
    if ret != 0:
        return False

    return True

def check_info_r():
    if not NEMU_HOME:
        return False

    cmd = ["info r", 'q']
    ret, output = run_nemu(cmd)
    if ret is None or output is None:
        return False
    if len(output[0]) <= 0:
        return False

    return True

def check_info_w():
    if not NEMU_HOME:
        return False

    cmd = ["w 0", "info w", 'q']
    ret, output = run_nemu(cmd)
    if ret is None or output is None:
        return False
    if len(output[0]) <= 0:
        return False

    return True 

def check_cmd_x():
    if not NEMU_HOME:
        return False

    cmd = ["x 10 0x80000000", 'q']
    ret, output = run_nemu(cmd)
    if ret is None or output is None:
        return False
    if len(output[0]) <= 0:
        return False

    return True

def check_cmd_si():
    if not NEMU_HOME:
        return False

    cmd = ["si 1", 'q']
    ret, output = run_nemu(cmd)
    if ret is None or output is None:
        return False
    if len(output[0]) <= 0:
        return False

    return True

def check_cmd_p():
    if not NEMU_HOME:
        return False

    cmd = ["p 1 + 2 * 3", 'q']
    ret, output = run_nemu(cmd)
    if ret is None or output is None:
        return False
    if len(output[0]) <= 0:
        return False

    return True


RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
RESET='\033[0m' # No Color

def main():
    check_list = [check_env, check_exit_gracefully, check_info_r, check_info_w, check_cmd_x, check_cmd_si, check_cmd_p]
    max_name_length = max(len(check.__name__) for check in check_list)
    for check in check_list:
        success = check()
        if success:
            print(f"{check.__name__:<{max_name_length}}: {GREEN}PASS{RESET}")
        else:
            print(f"{check.__name__:<{max_name_length}}: {RED  }FAIL{RESET}")



if __name__ == "__main__":
    sys.exit(main())
