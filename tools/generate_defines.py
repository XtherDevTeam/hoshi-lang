import os
import pathlib
import sys

language_version = 4

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
    src_content = pathlib.Path("share/defines.h").read_text()
    if get_first_6_digit_of_git_commit_hash() in src_content:
        print("defines.h is already up-to-date.")
        sys.exit(0)
        
    pathlib.Path("share/defines.h").write_text(header)