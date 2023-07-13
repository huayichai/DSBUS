#include <iostream>
#include "slice/slice.hpp"
#include "disk/disk_manager.hpp"
#include "gtest/gtest.h"

namespace dsbus {

TEST(DiskManagerTest, ConstructorTest) {
    remove("test.db");
    DiskManager disk_manager(Slice("test.db"), 128);
    EXPECT_EQ(disk_manager.GetPageNum(), 0);
    EXPECT_EQ(disk_manager.GetPageSize(), 128);
    disk_manager.ShutDown();
    remove("test.db");
}

TEST(DiskManagerTest, WriteHeaderPageTest) {
    remove("test.db");
    const size_t page_size = 20;
    char data[20] = "abcdefghijklmnop";
    {
        DiskManager disk_manager(Slice("test.db"), page_size);
        disk_manager.WritePage(0, data);
        disk_manager.WritePage(1, data);
        disk_manager.WritePage(2, data);
        disk_manager.WritePage(3, data);
        disk_manager.ShutDown();
    }
    {
        DiskManager disk_manager(Slice("test.db"), page_size);
        EXPECT_EQ(disk_manager.GetPageNum(), 4);
        disk_manager.ShutDown();
    }
    remove("test.db");
}

TEST(DiskManagerTest, ReadWriteTest) {
    remove("test.db");
    const size_t page_size = 20;
    char data[20] = "abcdefghijklmnop";
    char t_data[20] = "0000000000000000";
    {
        DiskManager disk_manager(Slice("test.db"), page_size);
        disk_manager.WritePage(0, data);
        disk_manager.ShutDown();
    }
    {
        DiskManager disk_manager(Slice("test.db"), page_size);
        disk_manager.ReadPage(0, t_data);
        EXPECT_EQ(strncmp(t_data, data, page_size), 0);
        disk_manager.ShutDown();
    }
    remove("test.db");
}

} // dsbus