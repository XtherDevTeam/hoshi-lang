//
// Created by XIaokang00010 on 2026/2/7.
//

#ifndef HOSHI_LANG_CODEGENOBJECTCACHE_HPP
#define HOSHI_LANG_CODEGENOBJECTCACHE_HPP

#include "compiler/ir/IR.h"
#include <memory>
#include <mutex>
#include <share/def.hpp>

namespace yoi {
    namespace serialization {
        template <typename T> void write(FILE *fp, const T &value) {
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable, otherwise explicit serialization method must be provided.");
            // general serialization helper for POD types
            fwrite(&value, sizeof(T), 1, fp);
        }

        template <typename T> void read(FILE *fp, T &value) {
            static_assert(std::is_trivially_copyable_v<T>,
                          "T must be trivially copyable, otherwise explicit deserialization method must be provided.");
            // general deserialization helper for POD types
            fread(&value, sizeof(T), 1, fp);
        }

        template <> void write(FILE *fp, const yoi::wstr &value);

        template <> void read(FILE *fp, yoi::wstr &value);
    } // namespace serialization

    struct CodegenObjectCacheEntry {
        // the absolute path of the source file on the disk
        yoi::wstr abs_path_on_disk;
        // the output object file name
        yoi::wstr object_filename;
        // the hash of the source file, if this file got removed in the next compilation, we can save this hash to free-list for reuse.
        yoi::indexT hash;
        // to further accelerate the compilation, we save the last modification time of the source file, if the last modification time is the same as
        // the last compilation, we can skip the codegen as they exposed the same implementations.
        yoi::indexT last_modification;

        CodegenObjectCacheEntry() = default;

        CodegenObjectCacheEntry setAbsPathOnDisk(const yoi::wstr &abs_path_on_disk);
        CodegenObjectCacheEntry setObjectFilename(const yoi::wstr &object_filename);
        CodegenObjectCacheEntry setHash(yoi::indexT hash);
        CodegenObjectCacheEntry setLastModification(yoi::indexT last_modification);

        const yoi::wstr &getAbsPathOnDisk() const;
        const yoi::wstr &getObjectFilename() const;
        yoi::indexT getHash() const;
        yoi::indexT getLastModification() const;
    };

    class CodegenObjectCache {
      public:
        std::map<yoi::wstr, CodegenObjectCacheEntry> cache;
        std::set<yoi::indexT> free_list;
        std::shared_ptr<IRBuildConfig> build_config;
        yoi::indexT next_hash = 0;
        mutable std::mutex cacheMutex;

        /**
         * @brief set the build config
         * @param build_config the build config
         * @return the CodegenObjectCache
         */
        CodegenObjectCache &setBuildConfig(const std::shared_ptr<IRBuildConfig> &build_config);

        /**
         * @brief purge the cache, add the entries that previously not in the cache, remove the entries that are not in the source_files
         * @param source_files the source files to check
         */
        void purge_and_update(const yoi::vec<yoi::wstr> &source_files);

        /**
         * @brief register a new entry to the cache
         * @param abs_path_on_disk the absolute path of the source file on the disk
         * @return the index of the new entry
         * @note the object filename will be generated based on the hash of the source file
         */
        yoi::indexT register_entry(const yoi::wstr &abs_path_on_disk);

        /**
         * @brief update the last modification time of an entry
         * @param abs_path_on_disk the absolute path of the source file on the disk
         * @param last_modification the last modification time
         */
        void update_last_modification(const yoi::wstr &abs_path_on_disk, yoi::indexT last_modification);

        /**
         * @brief get the entry from the cache
         * @param abs_path_on_disk the absolute path of the source file on the disk
         * @return the index of the entry
         */
        yoi::indexT get_entry_index(const yoi::wstr &abs_path_on_disk);

        /**
         * @brief get the entry from the cache
         * @param abs_path_on_disk the absolute path of the source file on the disk
         * @return the entry
         */
        CodegenObjectCacheEntry get_entry(const yoi::wstr &abs_path_on_disk);

        /**
         * @brief remove the entry from the cache
         * @param abs_path_on_disk the absolute path of the source file on the disk
         */
        void remove_entry(const yoi::wstr &abs_path_on_disk);

      private:
        yoi::indexT register_entry_unlocked(const yoi::wstr &abs_path_on_disk);
    };

    namespace serialization {
        template <> void write(FILE *fp, const CodegenObjectCache &value);

        template <> void read(FILE *fp, CodegenObjectCache &value);

        template <> void write(FILE *fp, const CodegenObjectCacheEntry &value);

        template <> void read(FILE *fp, CodegenObjectCacheEntry &value);

        template <> void write(FILE *fp, const std::set<yoi::indexT> &value);

        template <> void read(FILE *fp, std::set<yoi::indexT> &value);
    } // namespace serialization

} // namespace yoi
#endif