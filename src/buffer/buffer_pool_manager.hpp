#pragma once

#include <unordered_map>
#include <list>

#include "common/config.h"
#include "buffer/lru_replacer.hpp"
#include "disk/disk_page.hpp"
#include "disk/disk_manager.hpp"

namespace dsbus {

template<size_t page_size>
class BufferPoolManager {
public:
    /**
     *  @brief Init a buffer pool manager instance.
     *  
     *  @param pool_size buffer pool size
     *  @param disk_manager for read write db file
     */
    BufferPoolManager(const size_t pool_size, DiskManager *disk_manager)
                    : pool_size_(pool_size), disk_manager_(disk_manager) {
        next_page_id_ = disk_manager_->GetPageNum();
        pages_ = (disk::Page<page_size>*)malloc(page_size * pool_size_);
        replacer_ = new LRUReplacer(pool_size);
    }

    ~BufferPoolManager() {
        FlushAllData();
        free(pages_);
        delete replacer_;
    }

    size_t GetPoolSize() const { return pool_size_; }

    /**
     *  @brief Create a new page with page_id in the buffer pool.
     *  
     *  Remember to "Unpin" the page by calling bpm.UnpinPage() when the page not used, 
     *  so that lru_replacer wouldn't evict the page before the buffer pool manager "Unpin"s it.
     * 
     *  @return new alloc page pointer, if buffer pool is full, nullptr is returned.
     */
    disk::Page<page_size> *NewPage() {
        page_id_t new_page_id = AllocatePageID();
        frame_id_t frame_id;
        auto r = GetFreePage(&frame_id);
        if (!r) return nullptr;
        auto page = &pages_[frame_id];
        page->SetPageId(new_page_id);
        page->SetDirty(true);
        pages_map_[new_page_id] = frame_id;
        return page;
    }

    /**
     *  @brief  Fetch the requested page from the buffer pool. 
     *  
     *  Return nullptr if page_id needs to be fetched from the disk
     *  but all frames are currently in use and not evictable (in another word, pinned).
     * 
     *  Remember to "Unpin" the page by calling bpm.UnpinPage() when the page not used, 
     *  so that lru_replacer wouldn't evict the page before the buffer pool manager "Unpin"s it.
     *  
     *  @param page_id id of page to be fetched
     *  @return nullptr if page_id cannot be fetched, otherwise pointer to the requested page
     */
    disk::Page<page_size> *FetchPage(const page_id_t page_id) {
        frame_id_t frame_id;
        auto it = pages_map_.find(page_id);
        if (it != pages_map_.end()) {
            frame_id = it->second;
            replacer_->Pin(frame_id);
            return &pages_[frame_id];
        } else {
            auto r = GetFreePage(&frame_id); // replacer_.Victim() has Pin this page.
            if (!r) return nullptr;
            auto page = &pages_[frame_id];
            disk_manager_->ReadPage(page_id, (char *)page);
            page->SetDirty(false);
            pages_map_[page_id] = frame_id;
            return page;
        }
    }

    /**
     *  @brief Unpin the target page from the buffer pool. 
     *  
     *  If page_id is not in the buffer pool, return false.
     *  If its pin count is already 0, nothing happen, return true.
     *  Otherwise, decrement the pin count of a page.
     */
    bool UnpinPage(page_id_t page_id, bool is_dirty) {
        auto it = pages_map_.find(page_id);
        if (it == pages_map_.end()) return false;
        auto page = &pages_[it->second];
        if (is_dirty) page->SetDirty(is_dirty);
        replacer_->Unpin(it->second);
        return true;
    }

    /**
     *  @brief Flush all pages in buffer pool into disk. 
     */
    void FlushAllData() {
        for (auto kv : pages_map_) {
            auto page_id = kv.first;
            auto frame_id = kv.second;
            auto page = &pages_[frame_id];
            if (page->IsDirty()) {
                disk_manager_->WritePage(page_id, page->GetData());
                page->SetDirty(false);
            }
        }
    }

private:
    // buffer pool size
    size_t pool_size_;
    // Array of buffer pool pages
    disk::Page<page_size>* pages_;
    // lru_pilicy
    LRUReplacer* replacer_;
    // disk manager response for read-write pages from disk
    DiskManager *disk_manager_;
    // next alloc page id
    page_id_t next_page_id_;
    // map for page_id and frame_id
    std::unordered_map<page_id_t, frame_id_t> pages_map_;

    /**
     *  @brief Allocate a page on disk.
     */
    page_id_t AllocatePageID() { return next_page_id_++; }

    /**
     *  @brief Get free page.
     * 
     *  If free_list_ not empty, return a free page frame_id, 
     *  else evict least recent unused page, return it's frame_id.
     * 
     *  If there is no free page, return false and set frame_id nullptr.
     * 
     *  @param[out] frame_id the frame id of free page
     */
    bool GetFreePage(frame_id_t *frame_id) {
        auto r = replacer_->Victim(frame_id);
        if (!r) {
            frame_id = nullptr;
            return false;
        }
        auto page = &pages_[*frame_id];
        if (page->IsDirty()) {
            disk_manager_->WritePage(page->GetPageId(), page->GetData());
        }
        auto it = pages_map_.find(page->GetPageId());
        if (it != pages_map_.end()) {
            pages_map_.erase(it);
        }
        page->ResetMemory();
        return true;
    }
};


} // dsbus