#include <iostream>
#include "slice/slice.hpp"
#include "disk/disk_manager.hpp"
#include "buffer/buffer_pool_manager.hpp"
#include "gtest/gtest.h"

namespace dsbus {

TEST(BufferPoolManagerTest, ConstructorTest) {
    remove("test.db");
    const size_t page_size = 128;
    DiskManager disk_manager(Slice("test.db"), page_size);
    BufferPoolManager<page_size> bpm(16, &disk_manager);
    disk_manager.ShutDown();
    remove("test.db");
}

TEST(BufferPoolManagerTest, NewPageTest) {
    remove("test.db");
    const size_t page_size = 128;
    DiskManager disk_manager(Slice("test.db"), page_size);
    BufferPoolManager<page_size> bpm(2, &disk_manager);
    auto page1 = bpm.NewPage();
    EXPECT_EQ(page1->GetPageId(), 0);
    auto page2 = bpm.NewPage();
    EXPECT_EQ(page2->GetPageId(), 1);
    auto page3 = bpm.NewPage();
    EXPECT_EQ(page3->GetPageId(), 2);
    EXPECT_EQ(page1->GetPageId(), 2);
    disk_manager.ShutDown();
    remove("test.db");
}

TEST(BufferPoolManagerTest, FetchPageTest) {
    remove("test.db");
    const size_t page_size = 128;
    {
        DiskManager disk_manager(Slice("test.db"), page_size);
        BufferPoolManager<page_size> bpm(2, &disk_manager);
        auto page1 = bpm.NewPage();
        auto page2 = bpm.NewPage();
        auto c1 = page1->GetContent();
        memcpy(c1, "huayichai", 9);
        page1->SetDirty(true);
        auto c2 = page2->GetContent();
        memcpy(c2, "success", 7);
        page2->SetDirty(true);
        bpm.FlushAllData();
        disk_manager.ShutDown();
    }
    {
        DiskManager disk_manager(Slice("test.db"), page_size);
        BufferPoolManager<page_size> bpm(2, &disk_manager);
        EXPECT_EQ(disk_manager.GetPageNum(), 2);
        auto page1 = bpm.FetchPage(0);
        EXPECT_EQ(strncmp(page1->GetContent(), "huayichai", 9), 0);
        auto page2 = bpm.FetchPage(2);
        EXPECT_EQ(strncmp(page2->GetContent(), "success", 7), 0);
        disk_manager.ShutDown();
    }
    remove("test.db");
}

} // dsbus