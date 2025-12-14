//
// Created by XIaokang00010 on 2025/12/09.
//

#ifndef HOSHI_FS_H
#define HOSHI_FS_H

#include <cstdint>
#include <cstddef>
#include <cstdio>

#define LIBFS_EXPORT extern "C"

LIBFS_EXPORT bool runtime_fs_exists(const char* path);

LIBFS_EXPORT bool runtime_fs_isfile(const char* path);

LIBFS_EXPORT bool runtime_fs_isdir(const char* path);

LIBFS_EXPORT int64_t runtime_fs_get_mtime(const char* path);

LIBFS_EXPORT uint64_t runtime_fs_get_size(const char* path);

LIBFS_EXPORT int64_t runtime_fs_get_ctime(const char* path);

LIBFS_EXPORT int64_t runtime_fs_get_atime(const char* path);

LIBFS_EXPORT int runtime_fs_get_mode(const char* path);

LIBFS_EXPORT int runtime_fs_get_uid(const char* path);

LIBFS_EXPORT char *runtime_fs_temp_dir();

LIBFS_EXPORT char *runtime_fs_home_dir();

LIBFS_EXPORT char *runtime_fs_cwd();

LIBFS_EXPORT char *runtime_fs_realpath(const char* path);

LIBFS_EXPORT void runtime_fs_finalize(void *res);

LIBFS_EXPORT bool runtime_fs_mkdir(const char *path, int mode);

LIBFS_EXPORT bool runtime_fs_rmdir(const char* path);

LIBFS_EXPORT bool runtime_fs_rename(const char *old_path, const char *new_path);

LIBFS_EXPORT bool runtime_fs_remove(const char *path);

LIBFS_EXPORT void *runtime_fs_opendir(const char *name);

LIBFS_EXPORT char *runtime_fs_readdir(void *dir);

LIBFS_EXPORT void runtime_fs_closedir(void *dir);

#endif //HOSHI_FS_H