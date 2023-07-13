#pragma once

#include <unordered_map>
#include <list>

#include "disk/disk_page.hpp"
#include "disk/disk_manager.hpp"

namespace dsbus {

template<size_t page_size>
class BufferPoolManager {
public:
    BufferPoolManager(const size_t pool_size, DiskManager *disk_manager)
                    : pool_size_(pool_size), disk_manager_(disk_manager) {
        next_page_id_ = disk_manager_->GetPageNum();
        pages_ = (disk::Page<page_size>*)malloc(page_size * pool_size_);
        for (size_t i = 0; i < pool_size_; ++i) {
            free_list_.push_back((frame_id_t)i);
        }
    }

    ~BufferPoolManager() {
        FlushAllData();
        free(pages_);
    }

    size_t GetPoolSize() const { return pool_size_; }

    disk::Page<page_size> *NewPage() {
        page_id_t new_page_id = AllocatePageID();
        frame_id_t frame_id = GetFreePage(new_page_id);
        return &pages_[frame_id];
    }

    disk::Page<page_size> *FetchPage(const page_id_t page_id) {
        frame_id_t frame_id;
        auto it = pages_map_.find(page_id);
        if (it != pages_map_.end()) {
            frame_id = it->second;
            auto it = lru_map_.find(frame_id)->second;
            lru_list_.erase(it);
            lru_list_.push_front(frame_id);
            lru_map_[frame_id] = lru_list_.begin();
        } else {
            frame_id = GetFreePage(page_id);
        }
        disk_manager_->ReadPage(page_id, (char *)&pages_[frame_id]);
        pages_[frame_id].SetDirty(false);
        return &pages_[frame_id];
    }

    void FlushAllData() {
        for (auto kv : pages_map_) {
            auto page_id = kv.first;
            auto frame_id = kv.second;
            auto page = &pages_[frame_id];
            if (page->IsDirty()) {
                disk_manager_->WritePage(page_id, page->GetData());
            }
        }
        disk_manager_->ShutDown();
    }

private:
    // buffer pool size
    size_t pool_size_;
    // Array of buffer pool pages
    disk::Page<page_size>* pages_;
    // disk manager response for read-write pages from disk
    DiskManager *disk_manager_;
    page_id_t next_page_id_;
    
    // LRU
    using frame_id_t = int32_t;
    // map for page_id and frame_id
    std::unordered_map<page_id_t, frame_id_t> pages_map_;
    // unused page in pages_
    std::list<frame_id_t> free_list_;
    // least recent used list
    std::list<frame_id_t> lru_list_;
    std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> lru_map_;

    /**
     *  @brief Allocate a page on disk.
     */
    page_id_t AllocatePageID() { return next_page_id_++; }

    /**
     *  @brief Get free page.
     * 
     *  if free_list_ not empty, return a free page frame_id
     *  else evict least recent unused page, return it's frame_id
     *  @param page_id set free page id
     */
    frame_id_t GetFreePage(const page_id_t page_id) {
        frame_id_t frame_id;
        disk::Page<page_size>* page;
        // have free page
        if (!free_list_.empty()) {
            frame_id = free_list_.front();
            free_list_.pop_front();
            page = &pages_[frame_id];
        } else { // evict least recent unused page
            frame_id = lru_list_.back();
            lru_list_.pop_back();
            page = &pages_[frame_id];
            auto old_page_id = page->GetPageId();
            if (page->IsDirty()) {
                disk_manager_->WritePage(old_page_id, page->GetData());
            }
            pages_map_.erase(pages_map_.find(old_page_id));
        }
        page->ResetMemory();
        page->SetDirty(true);
        page->SetPageId(page_id);
        pages_map_[page_id] = frame_id;
        
        lru_list_.push_front(frame_id);
        lru_map_[frame_id] = lru_list_.begin();
        return frame_id;
    }
};


} // dsbus