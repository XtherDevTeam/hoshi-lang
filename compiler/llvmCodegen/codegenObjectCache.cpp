//
// Created by XIaokang00010 on 2026/2/7.
//

#include "codegenObjectCache.hpp"
#include "share/def.hpp"
#include <cstdio>
#include <filesystem>

namespace yoi {
    namespace serialization {
        template <> void write(FILE *fp, const yoi::wstr &value) {
            // general serialization helper for wstr
            // read length first
            write<uint64_t>(fp, value.size());
            fwrite(value.data(), sizeof(yoi::wstr::value_type), value.size(), fp);
        }

        template <> void read(FILE *fp, yoi::wstr &value) {
            // general deserialization helper for wstr
            // read length first
            uint64_t len;
            read<uint64_t>(fp, len);
            value.resize(len);
            fread(value.data(), sizeof(yoi::wstr::value_type), len, fp);
        }

        template <> void write(FILE *fp, const CodegenObjectCacheEntry &value) {
            write(fp, value.abs_path_on_disk);
            write(fp, value.object_filename);
            write(fp, value.hash);
            write(fp, value.last_modification);
        }

        template <> void read(FILE *fp, CodegenObjectCacheEntry &value) {
            read(fp, value.abs_path_on_disk);
            read(fp, value.object_filename);
            read(fp, value.hash);
            read(fp, value.last_modification);
        }

        template <> void write(FILE *fp, const CodegenObjectCache &value) {
            write<uint64_t>(fp, value.cache.size());
            for (auto &item : value.cache) {
                write(fp, item.first);
                write(fp, item.second);
            }
            write<uint64_t>(fp, value.free_list.size());
            for (auto &item : value.free_list) {
                write(fp, item);
            }
            write<yoi::indexT>(fp, value.next_hash);
        }

        template <> void read(FILE *fp, CodegenObjectCache &value) {
            uint64_t len;
            read<uint64_t>(fp, len);
            for (auto i = 0; i < len; i++) {
                yoi::wstr key;
                CodegenObjectCacheEntry entry;
                read(fp, key);
                read(fp, entry);
                value.cache.insert({key, entry});
            }
            read<uint64_t>(fp, len);
            value.free_list.reserve(len);
            for (auto i = 0; i < len; i++) {
                yoi::indexT item;
                read(fp, item);
                value.free_list.push_back(item);
            }
            read<yoi::indexT>(fp, value.next_hash);
        }
    } // namespace serialization

    CodegenObjectCacheEntry CodegenObjectCacheEntry::setAbsPathOnDisk(const yoi::wstr &abs_path_on_disk) {
        this->abs_path_on_disk = abs_path_on_disk;
        return *this;
    }

    CodegenObjectCacheEntry CodegenObjectCacheEntry::setObjectFilename(const yoi::wstr &object_filename) {
        this->object_filename = object_filename;
        return *this;
    }

    CodegenObjectCacheEntry CodegenObjectCacheEntry::setHash(yoi::indexT hash) {
        this->hash = hash;
        return *this;
    }

    const yoi::wstr &CodegenObjectCacheEntry::getAbsPathOnDisk() const {
        return abs_path_on_disk;
    }

    const yoi::wstr &CodegenObjectCacheEntry::getObjectFilename() const {
        return object_filename;
    }

    yoi::indexT CodegenObjectCacheEntry::getHash() const {
        return hash;
    }

    void CodegenObjectCache::purge_and_update(const yoi::vec<yoi::wstr> &source_files) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        std::set<yoi::wstr> source_set(source_files.begin(), source_files.end());
        yoi::vec<yoi::wstr> to_be_removed, to_be_added;
        for (auto &item : cache) {
            if (source_set.find(item.first) == source_set.end()) {
                to_be_removed.push_back(item.first);
            }
        }
        for (auto &item : source_files) {
            if (cache.find(item) == cache.end()) {
                to_be_added.push_back(item);
            }
        }
        for (auto &item : to_be_removed) {
            cache.erase(item);
        }
        for (auto &item : to_be_added) {
            register_entry(item);
        }
    }

    yoi::indexT CodegenObjectCache::register_entry_unlocked(const yoi::wstr &abs_path_on_disk) {
        // check if the entry is already in the cache
        if (cache.find(abs_path_on_disk) != cache.end()) {
            return cache[abs_path_on_disk].hash;
        }
        yoi::indexT hash = 0;
        if (free_list.empty()) {
            hash = next_hash++;
        } else {
            hash = free_list.back();
            free_list.pop_back();
        }
        if (!std::filesystem::exists(build_config->buildCachePath)) {
            std::filesystem::create_directories(build_config->buildCachePath);
        }
        yoi::wstr object_filename = build_config->buildCachePath + L"/" + std::to_wstring(hash) + L".o";
        printf("Creating cache for file: %s\n", yoi::wstring2string(abs_path_on_disk).c_str());
        cache[abs_path_on_disk] = CodegenObjectCacheEntry().setAbsPathOnDisk(abs_path_on_disk).setObjectFilename(object_filename).setHash(hash).setLastModification(0);
        return hash;
    }

    yoi::indexT CodegenObjectCache::register_entry(const yoi::wstr &abs_path_on_disk) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        return register_entry_unlocked(abs_path_on_disk);
    }

    yoi::indexT CodegenObjectCache::get_entry_index(const yoi::wstr &abs_path_on_disk) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        if (cache.find(abs_path_on_disk) == cache.end()) {
            return -1;
        }
        return cache[abs_path_on_disk].hash;
    }

    CodegenObjectCacheEntry CodegenObjectCache::get_entry(const yoi::wstr &abs_path_on_disk) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        if (cache.find(abs_path_on_disk) == cache.end()) {
            register_entry_unlocked(abs_path_on_disk);
        }
        return cache.at(abs_path_on_disk);
    }

    void CodegenObjectCache::remove_entry(const yoi::wstr &abs_path_on_disk) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        free_list.push_back(cache.at(abs_path_on_disk).hash);
        cache.erase(abs_path_on_disk);
    }

    CodegenObjectCache &CodegenObjectCache::setBuildConfig(const std::shared_ptr<IRBuildConfig> &build_config) {
        this->build_config = build_config;
        return *this;
    }

    void CodegenObjectCache::update_last_modification(const yoi::wstr &abs_path_on_disk, yoi::indexT last_modification) {
        std::lock_guard<std::mutex> lock(cacheMutex);
        if (cache.find(abs_path_on_disk) != cache.end()) {
            cache[abs_path_on_disk].last_modification = last_modification;
        }
    }

    CodegenObjectCacheEntry CodegenObjectCacheEntry::setLastModification(yoi::indexT last_modification) {
        this->last_modification = last_modification;
        return *this;
    }

    yoi::indexT CodegenObjectCacheEntry::getLastModification() const {
        return last_modification;
    }
} // namespace yoi