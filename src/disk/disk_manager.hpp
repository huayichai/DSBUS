#pragma once
#include <iostream>
#include <fstream>

#include "slice/slice.hpp"
#include "disk/disk_config.h"
#include "disk/disk_page.hpp"

namespace dsbus {

/**
 *  DiskManager takes care of the allocation and deallocation of pages within a storage engine. It performs the reading and
 *  writing of pages to and from disk, providing a logical file layer within the context of a storage engine.
 *  
 *  Now one DiskManager instance only support manage single file.
 *  Structure: HeaderPage(16B) + Page * N
 *  
 */
class DiskManager {
public:
    /**
     *  @brief Construct a disk manager instance.
     *  
     *  @param db_file_name db file path.
     *  @param page_size the size of page in the db file.
     */
    DiskManager(const Slice &db_file_name, const size_t page_size) : db_file_name_(db_file_name) {
        db_io_.open(db_file_name.Data(), std::ios::binary | std::ios::in | std::ios::out);
        // directory or file does not exist
        if (!db_io_.is_open()) {
            db_io_.clear();
            // create a new file
            db_io_.open(db_file_name.Data(), std::ios::binary | std::ios::trunc | std::ios::out | std::ios::in);
            if (!db_io_.is_open()) {
                std::cerr << "can't open db file" << std::endl;
                exit(0);
            }
            header_page_.page_num_ = 0;
            header_page_.page_size_ = page_size;
            WriteHeaderPage();
            return;
        }
        ReadHeaderPage();
    }

    ~DiskManager() { ShutDown(); }

    void ShutDown() { WriteHeaderPage(); db_io_.close(); }

    void ReadPage(page_id_t page_id, char *page_data) {
        size_t offset = disk::DISK_HEADER_PAGE_SIZE + (size_t)page_id * header_page_.page_size_;
        ReadDisk(offset, page_data, header_page_.page_size_);
    }

    void WritePage(page_id_t page_id, const char *page_data) {
        size_t offset = disk::DISK_HEADER_PAGE_SIZE + (size_t)page_id * header_page_.page_size_;
        WriteDisk(offset, page_data, header_page_.page_size_);
        // update header_page_.page_num_, but not flush to disk immediately.
        header_page_.page_num_ = header_page_.page_num_ > (size_t)page_id ? header_page_.page_num_ : page_id + 1;
    }

    size_t GetPageSize() const { return header_page_.page_size_; }

    size_t GetPageNum() const { return header_page_.page_num_; }

private:
    const Slice db_file_name_;
    disk::DiskHeaderPage header_page_;
    std::fstream db_io_;

    void WriteDisk(const size_t offset, const char *data, const size_t data_size) {
        db_io_.seekp(offset);
        db_io_.write(data, data_size);
        // check for I/O error
        if (db_io_.bad()) {
            std::cerr << "I/O error while writing" << std::endl;
            exit(0);
        }
        // needs to flush to keep disk file in sync
        db_io_.flush();
    }

    void ReadDisk(const size_t offset, char *data, const size_t data_size) {
        auto file_size = header_page_.GetFileSize();
        if (offset + data_size > file_size) {
            std::cerr << "I/O error reading past end of file" << std::endl;
            exit(0);
        }
        db_io_.seekp(offset);
        db_io_.read(data, data_size);
        if (db_io_.bad()) {
            std::cerr << "I/O error while reading" << std::endl;
            exit(0);
        }
    }

    void ReadHeaderPage() { ReadDisk(0, (char *)&header_page_, disk::DISK_HEADER_PAGE_SIZE); }

    void WriteHeaderPage() { WriteDisk(0, (const char *)&header_page_, disk::DISK_HEADER_PAGE_SIZE); }
};


} // dsbus