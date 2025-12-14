/*
// Created by XIaokang00010 on 2025/12/09.
*/

// Ensure standard functions like strdup, realpath, etc., are exposed

#ifdef _WIN32
#include <windows.h>
#include <cstring>

struct DirectoryHandle {
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAA findFileData;
    bool firstEntry = true;
};

#else
#include <dirent.h> // POSIX header for directory operations
#endif

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
    DWORD len = GetTempPathA(0, nullptr);
    if (len == 0) return nullptr;
    
    char* buf = (char*)malloc(len + 1);
    if (!buf) return nullptr;
    
    if (GetTempPathA(len + 1, buf) == 0) {
        free(buf);
        return nullptr;
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
    return nullptr;
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
    return nullptr;
#endif
}

char *runtime_fs_cwd() {
    // Portable way to get CWD without guessing buffer size
    // Start with a reasonable size, typically 1024 or 4096
    size_t size = 1024;
    char* buf = (char*)malloc(size);
    
    if (!buf) return nullptr;

    while (getcwd(buf, (int)size) == nullptr) {
        if (errno == ERANGE) {
            size *= 2;
            char* new_buf = (char*)realloc(buf, size);
            if (!new_buf) {
                free(buf);
                return nullptr;
            }
            buf = new_buf;
        } else {
            free(buf);
            return nullptr;
        }
    }
    
    // Optional: Trim unused memory
    char* final_buf = strdup(buf);
    free(buf);
    return final_buf;
}

char *runtime_fs_realpath(const char* path) {
    if (!path) return nullptr;
#ifdef _WIN32
    // _fullpath with nullptr automatically mallocs
    return _fullpath(nullptr, path, 0); 
#else
    // realpath with nullptr automatically mallocs (POSIX.1-2008)
    return realpath(path, nullptr); 
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

bool runtime_fs_remove(const char *path) {
    return remove(path) == 0;
}

bool runtime_fs_rename(const char *old_path, const char *new_path) {
    return rename(old_path, new_path) == 0;
}

void *runtime_fs_opendir(const char *name) {
#ifdef _WIN32
    char *concated = (char*)malloc(strlen(name) + 3);
    if (!concatenated) return nullptr;
    strcpy(concatenated, name);
    strcat(concatenated, "/*");
    
    DirectoryHandle *dir = (DirectoryHandle*)malloc(sizeof(DirectoryHandle));
    if (!dir) {
        free(concatenated);
        return nullptr;
    }
    dir->hFind = FindFirstFileA(concatenated, &(dir->findFileData));

    if (dir->hFind == INVALID_HANDLE_VALUE) {
        free(dir);
        free(concatenated);
        return nullptr;
    }
    dir->firstEntry = true;
    return dir;
#else
    return opendir(name);
#endif
}

char *runtime_fs_readdir(void *dir) {
    if (!dir) return nullptr;

#ifdef _WIN32
    DirectoryHandle *handle = (DirectoryHandle*)dir;

    if (handle->firstEntry) {
        handle->firstEntry = false;
        return handle->findFileData.cFileName;
    }

    if (FindNextFileA(handle->hFind, &(handle->findFileData)) != 0) {
        return handle->findFileData.cFileName;
    }
    return nullptr;
#else
    struct dirent* entry = readdir((DIR*)dir);
    if (!entry) {
        return nullptr;
    }
    return entry->d_name;
#endif
}

void runtime_fs_closedir(void *dir) {
    if (!dir) return;

#ifdef _WIN32
    DirectoryHandle *handle = (DirectoryHandle*)dir;
    if (handle->hFind != INVALID_HANDLE_VALUE) {
        FindClose(handle->hFind);
    }
    delete handle;
#else
    closedir((DIR*)dir);
#endif
}