#pragma once
#include <iostream>
#include "disk/disk_config.h"

namespace dsbus {

namespace disk {

const size_t DISK_HEADER_PAGE_SIZE = 16;

/**
 *  The first page in a db file.
 */
class DiskHeaderPage {
public:
    size_t page_size_;
    size_t page_num_;

    DiskHeaderPage() : DiskHeaderPage(0, 0) {}

    DiskHeaderPage(const size_t page_size, const size_t page_num)
                  : page_size_(page_size), page_num_(page_num) {}
    
    DiskHeaderPage(const size_t page_size)
                  : page_size_(page_size), page_num_(0) {}

    /**
     *  @brief Returns the size of file.
     * 
     *  in byte
     */
    size_t GetFileSize() {
        return DISK_HEADER_PAGE_SIZE + page_size_ * page_num_;
    }    
};

/**
 *  Original page in db file.
 */
template<size_t page_size>
class Page {
public:
    Page() {
        static_assert(page_size > SIZE_PAGE_HEADER);
        // data_ = (char *)malloc(sizeof(char) * page_size);
        ResetMemory();
        SetPageId(INVALID_PAGE_ID);
    }

    ~Page() {
        free(data_);
    }

    inline char *GetData() { return data_; }
    inline void SetData(const char *s, const size_t size) {
        if (page_size != size) {
            std::cerr << "Out of page size." << std::endl;
            exit(0);
        }
        memcpy(data_, s, page_size);
        SetDirty(true);
    }

    inline char *GetContent() { return data_ + SIZE_PAGE_HEADER; }

    inline page_id_t GetPageId() { return *(page_id_t *)(data_ + OFFSET_LSN); }
    inline void SetPageId(const page_id_t page_id) { *(page_id_t *)(data_ + OFFSET_LSN) = page_id; }

    inline void SetDirty(bool dirty) { is_dirty_ = dirty; }

    inline bool IsDirty() const { return is_dirty_; }

    inline void ResetMemory() { memset(data_, OFFSET_PAGE_START, page_size); }

private:
protected:
    static constexpr size_t SIZE_PAGE_HEADER = 8;
    static constexpr size_t OFFSET_PAGE_START = 0;
    static constexpr size_t OFFSET_LSN = 4;

protected:
    char data_[page_size];
    bool is_dirty_ = false;
};

} // disk


} // dsbus