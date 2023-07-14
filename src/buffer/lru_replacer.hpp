#pragma once

#include <iostream>
#include <list>
#include <unordered_map>

#include "common/config.h"

namespace dsbus {
/**
 * LRUReplacer implements the Least Recently Used replacement policy.
 */
class LRUReplacer {
public:
    /**
     *  @brief Create a new LRUReplacer.
     *  @param num_pages the maximum number of pages the LRUReplacer will be required to store
     */
    explicit LRUReplacer(size_t num_pages) {
        for (size_t i = 0; i < num_pages; ++i) {
            free_list_.push_back((frame_id_t)i);
            free_map_[(frame_id_t)i] = std::prev(free_list_.end());
        }
    }

    ~LRUReplacer() {}

    /**
     *  @brief Remove the victim frame as defined by the replacement policy.
     *         Pin the evicted frame.
     *  @param[out] frame_id id of frame that was removed, nullptr if no victim was found
     *  @return true if a victim frame was found, false otherwise
     */
    bool Victim(frame_id_t *frame_id) {
        // select evicted frame
        if (!free_list_.empty()) {
            *frame_id = free_list_.front();
            free_list_.pop_front();
            free_map_.erase(free_map_.find(*frame_id));
        } else if (!lru_list_.empty()) {
            *frame_id = lru_list_.front();
            lru_list_.pop_front();
            lru_map_.erase(*frame_id);
        } else {
            frame_id = nullptr;
            return false;
        }
        // pin frame
        auto it = pin_map_.find(*frame_id);
        if (it == pin_map_.end()) {
            pin_map_[*frame_id] = 1;
        } else {
            std::cerr << "frame both exist in lru_list and pin_map" << std::endl;
            exit(0);
        }
        return true;
    }

    /**
     *  @brief Pins a frame, indicating that it should not be victimized until it is unpinned.
     *  @param frame_id the id of the frame to pin
     */
    void Pin(frame_id_t frame_id) {
        auto it1 = lru_map_.find(frame_id);
        auto it2 = pin_map_.find(frame_id);
        if (it1 == lru_map_.end() && it2 == pin_map_.end()) {
            auto it3 = free_map_.find(frame_id);
            if (it3 != free_map_.end()) {
                free_list_.erase(it3->second);
                free_map_.erase(it3);
                pin_map_[frame_id] = 1;
            }
            return;
        }
        if (it1 != lru_map_.end()) {
            lru_list_.erase(it1->second);
            lru_map_.erase(it1);
        }
        if (it2 != pin_map_.end()) {
            ++it2->second;
        } else {
            pin_map_[frame_id] = 1;
        }
    }

    /**
     *  @brief Unpins a frame, indicating that it can now be victimized.
     *  @param frame_id the id of the frame to unpin
     */
    void Unpin(frame_id_t frame_id) {
        auto it1 = pin_map_.find(frame_id);
        auto it2 = lru_map_.find(frame_id);
        if (it1 == pin_map_.end() && it2 == lru_map_.end()) return;
        if (it1 == pin_map_.end()) return;
        --it1->second;
        if (it1->second != 0) return;
        pin_map_.erase(it1);
        if (it2 == lru_map_.end()) {
            lru_list_.push_back(frame_id);
            lru_map_[frame_id] = std::prev(lru_list_.end());
        } else {
            std::cerr << "frame both exist in lru_list and pin_map" << std::endl;
            exit(0);
        }
    }

    /**
     *  @brief The number of elements in the replacer that can be victimized.
     */
    size_t Size() { return free_list_.size() + lru_list_.size(); }

private:
    std::list<frame_id_t> free_list_;
    std::list<frame_id_t> lru_list_;
    std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> free_map_;
    std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> lru_map_;
    std::unordered_map<frame_id_t, size_t> pin_map_;
    
};

} // dsbus