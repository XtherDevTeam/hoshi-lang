//
// Created by XIaokang00010 on 2026/2/7.
//

#include "codegenObjectCache.hpp"

namespace yoi {
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

    yoi::indexT CodegenObjectCache::register_entry(const yoi::wstr &abs_path_on_disk) {
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
        yoi::wstr object_filename = build_config->buildCachePath + L"/" + std::to_wstring(hash) + L".o";
        cache[abs_path_on_disk] = CodegenObjectCacheEntry().setAbsPathOnDisk(abs_path_on_disk).setObjectFilename(object_filename).setHash(hash);
        return hash;
    }

    yoi::indexT CodegenObjectCache::get_entry_index(const yoi::wstr &abs_path_on_disk) {
        if (cache.find(abs_path_on_disk) == cache.end()) {
            return -1;
        }
        return cache[abs_path_on_disk].hash;
    }

    CodegenObjectCacheEntry CodegenObjectCache::get_entry(const yoi::wstr &abs_path_on_disk) {
        return cache.at(abs_path_on_disk);
    }

    void CodegenObjectCache::remove_entry(const yoi::wstr &abs_path_on_disk) {
        free_list.push_back(cache.at(abs_path_on_disk).hash);
        cache.erase(abs_path_on_disk);
    }

    CodegenObjectCache CodegenObjectCache::setBuildConfig(const std::shared_ptr<IRBuildConfig> &build_config) {
        this->build_config = build_config;
        return *this;
    }
    
    CodegenObjectCacheEntry CodegenObjectCacheEntry::setLastModification(yoi::indexT last_modification) {
        this->last_modification = last_modification;
        return *this;
    }

    yoi::indexT CodegenObjectCacheEntry::getLastModification() const {
        return last_modification;
    }
} // namespace yoi