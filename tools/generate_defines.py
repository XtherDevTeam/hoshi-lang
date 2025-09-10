import os
import sys

language_version = "0"

def get_first_6_digit_of_git_commit_hash():
    return os.popen('git rev-parse --short HEAD').read().strip()[:6]

header = f"""
#ifndef HOSHI_LANG_DEFINES_H
#define HOSHI_LANG_DEFINES_H

#define HOSHI_LANG_VERSION "{language_version}"
#define HOSHI_LANG_GIT_COMMIT_HASH "{get_first_6_digit_of_git_commit_hash()}"

#endif
"""

if __name__ == "__main__":
    with open("share/defines.h", "w") as f:
        f.write(header)