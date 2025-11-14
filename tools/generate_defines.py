import os
import pathlib
import sys

def write_version_number(version_number: int):
    pathlib.Path("tools/VERSION").write_text(str(version_number))
    return version_number

def get_version_number():
    language_version = int(pathlib.Path("tools/VERSION").read_text().strip()) if pathlib.Path("tools/VERSION").exists() else 11
    return write_version_number(language_version + 1)

def get_first_6_digit_of_git_commit_hash():
    return os.popen('git rev-parse --short HEAD').read().strip()[:6]

if __name__ == "__main__":
    src_content = pathlib.Path("share/defines.h").read_text()
    if get_first_6_digit_of_git_commit_hash() in src_content:
        print("defines.h is already up-to-date.")
        sys.exit(0)
        
    header = f"""
#ifndef HOSHI_LANG_DEFINES_H
#define HOSHI_LANG_DEFINES_H

#define HOSHI_LANG_VERSION "{get_version_number()}"
#define HOSHI_LANG_GIT_COMMIT_HASH "{get_first_6_digit_of_git_commit_hash()}"

#endif
"""
    pathlib.Path("share/defines.h").write_text(header.strip())