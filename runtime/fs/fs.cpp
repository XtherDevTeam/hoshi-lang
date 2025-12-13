/*
// Created by XIaokang00010 on 2025/12/09.
*/

// Ensure standard functions like strdup, realpath, etc., are exposed
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#define _CRT_SECURE_NO_WARNINGS // For MSVC

#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
    
    // Windows specific types and macros for 64-bit file support
    #define stat_struct struct __stat64
    #define stat_func _stat64
    #define strdup _strdup
    #define getcwd _getcwd
    
    // Windows doesn't always define standard POSIX file type macros
    #ifndef S_ISREG
        #define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
    #endif
    #ifndef S_ISDIR
        #define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
    #endif
    
    #define PATH_SEPARATOR '\\'
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
    #include <limits.h>
    
    #define stat_struct struct stat
    #define stat_func stat
    #define PATH_SEPARATOR '/'
#endif


static int get_stat(const char* path, stat_struct* buf) {
    if (!path || !*path) return -1;
    return stat_func(path, buf);
}



bool runtime_fs_exists(const char* path) {
#ifdef _WIN32
    return _access(path, 0) == 0;
#else
    return access(path, F_OK) == 0;
#endif
}

bool runtime_fs_isfile(const char* path) {
    stat_struct s;
    if (get_stat(path, &s) != 0) return false;
    return S_ISREG(s.st_mode);
}

bool runtime_fs_isdir(const char* path) {
    stat_struct s;
    if (get_stat(path, &s) != 0) return false;
    return S_ISDIR(s.st_mode);
}



int64_t runtime_fs_get_mtime(const char* path) {
    stat_struct s;
    if (get_stat(path, &s) != 0) return -1;
    return (int64_t)s.st_mtime;
}

uint64_t runtime_fs_get_size(const char* path) {
    stat_struct s;
    if (get_stat(path, &s) != 0) return 0;
    return (uint64_t)s.st_size;
}

int64_t runtime_fs_get_ctime(const char* path) {
    stat_struct s;
    if (get_stat(path, &s) != 0) return -1;
    return (int64_t)s.st_ctime;
}

int64_t runtime_fs_get_atime(const char* path) {
    stat_struct s;
    if (get_stat(path, &s) != 0) return -1;
    return (int64_t)s.st_atime;
}

int runtime_fs_get_uid(const char* path) {
    stat_struct s;
    if (get_stat(path, &s) != 0) return -1;
    return (int)s.st_uid; // Note: On Windows this is usually 0
}



char *runtime_fs_temp_dir() {
#ifdef _WIN32
    DWORD len = GetTempPathA(0, NULL);
    if (len == 0) return NULL;
    
    char* buf = (char*)malloc(len + 1);
    if (!buf) return NULL;
    
    if (GetTempPathA(len + 1, buf) == 0) {
        free(buf);
        return NULL;
    }
    
    // Remove trailing backslash if present (consistency preference)
    size_t actual_len = strlen(buf);
    if (actual_len > 0 && buf[actual_len - 1] == '\\') {
        buf[actual_len - 1] = '\0';
    }
    return buf;
#else
    const char* env_temp = getenv("TMPDIR");
    if (!env_temp) env_temp = getenv("TMP");
    if (!env_temp) env_temp = getenv("TEMP");
    if (!env_temp) env_temp = getenv("TEMPDIR");
    if (!env_temp) env_temp = "/tmp";
    
    return strdup(env_temp);
#endif
}

char *runtime_fs_home_dir() {
#ifdef _WIN32
    const char* drive = getenv("HOMEDRIVE");
    const char* path = getenv("HOMEPATH");
    const char* userprofile = getenv("USERPROFILE");

    if (userprofile) {
        return strdup(userprofile);
    } else if (drive && path) {
        size_t len = strlen(drive) + strlen(path) + 1;
        char* buf = (char*)malloc(len);
        if (buf) {
            sprintf(buf, "%s%s", drive, path);
        }
        return buf;
    }
    return NULL;
#else
    const char* home = getenv("HOME");
    if (home) {
        return strdup(home);
    }
    
    // Fallback using password database
    struct passwd* pwd = getpwuid(getuid());
    if (pwd) {
        return strdup(pwd->pw_dir);
    }
    return NULL;
#endif
}

char *runtime_fs_cwd() {
    // Portable way to get CWD without guessing buffer size
    // Start with a reasonable size, typically 1024 or 4096
    size_t size = 1024;
    char* buf = (char*)malloc(size);
    
    if (!buf) return NULL;

    while (getcwd(buf, (int)size) == NULL) {
        if (errno == ERANGE) {
            size *= 2;
            char* new_buf = (char*)realloc(buf, size);
            if (!new_buf) {
                free(buf);
                return NULL;
            }
            buf = new_buf;
        } else {
            free(buf);
            return NULL;
        }
    }
    
    // Optional: Trim unused memory
    char* final_buf = strdup(buf);
    free(buf);
    return final_buf;
}

char *runtime_fs_realpath(const char* path) {
    if (!path) return NULL;
#ifdef _WIN32
    // _fullpath with NULL automatically mallocs
    return _fullpath(NULL, path, 0); 
#else
    // realpath with NULL automatically mallocs (POSIX.1-2008)
    return realpath(path, NULL); 
#endif
}

void runtime_fs_finalize(void *res) {
    if (res) {
        free(res);
    }
}

bool runtime_fs_mkdir(const char *path, int mode) {
    if (!path) return false;
#ifdef _WIN32
    return _mkdir(path) == 0;
#else
    return mkdir(path, mode) == 0;
#endif
}

bool runtime_fs_rmdir(const char *path) {
    if (!path) return false;
    return rmdir(path) == 0;
}