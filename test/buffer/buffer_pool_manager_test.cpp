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
    
    auto page1 = bpm.NewPage(); EXPECT_EQ(page1->GetPageId(), 0);
    auto page2 = bpm.NewPage(); EXPECT_EQ(page2->GetPageId(), 1);

    EXPECT_EQ(bpm.NewPage(), nullptr);

    disk_manager.ShutDown();
    remove("test.db");
}

TEST(BufferPoolManagerTest, UnpinPageTest) {
    remove("test.db");
    const size_t page_size = 128;
    DiskManager disk_manager(Slice("test.db"), page_size);
    BufferPoolManager<page_size> bpm(2, &disk_manager);
    
    auto page1 = bpm.NewPage(); EXPECT_EQ(page1->GetPageId(), 0);
    auto page2 = bpm.NewPage(); EXPECT_EQ(page2->GetPageId(), 1);

    auto r = bpm.UnpinPage(page1->GetPageId(), false); EXPECT_EQ(true, r);

    auto page3 = bpm.NewPage(); EXPECT_EQ(page3->GetPageId(), 2);

    disk_manager.ShutDown();
    remove("test.db");
}

TEST(BufferPoolManagerTest, FetchPageTest1) {
    remove("test.db");
    const size_t page_size = 128;
    {
        DiskManager disk_manager(Slice("test.db"), page_size);
        BufferPoolManager<page_size> bpm(2, &disk_manager);
        
        auto page1 = bpm.NewPage();
        auto page2 = bpm.NewPage();
        memcpy(page1->GetContent(), "huayichai", 9);
        memcpy(page2->GetContent(), "success", 7);
        
        bpm.FlushAllData();
        disk_manager.ShutDown();
    }
    {
        DiskManager disk_manager(Slice("test.db"), page_size);
        BufferPoolManager<page_size> bpm(2, &disk_manager);
        EXPECT_EQ(disk_manager.GetPageNum(), 2);
        auto page1 = bpm.FetchPage(0);
        EXPECT_EQ(strncmp(page1->GetContent(), "huayichai", 9), 0);
        auto page2 = bpm.FetchPage(1);
        EXPECT_EQ(strncmp(page2->GetContent(), "success", 7), 0);
        disk_manager.ShutDown();
    }
    remove("test.db");
}

TEST(BufferPoolManagerTest, FetchPageTest2) {
    remove("test.db");
    const size_t page_size = 128;
    DiskManager disk_manager(Slice("test.db"), page_size);
    BufferPoolManager<page_size> bpm(2, &disk_manager);
    
    auto page1 = bpm.NewPage(); EXPECT_EQ(page1->GetPageId(), 0);
    auto page2 = bpm.NewPage(); EXPECT_EQ(page2->GetPageId(), 1);
    memcpy(page1->GetContent(), "test1", 5);
    memcpy(page2->GetContent(), "test2", 5);

    bpm.UnpinPage(page1->GetPageId(), true);
    bpm.UnpinPage(page2->GetPageId(), true);

    auto page3 = bpm.NewPage(); EXPECT_EQ(page3->GetPageId(), 2);
    memcpy(page3->GetContent(), "test3", 5);
    bpm.UnpinPage(page3->GetPageId(), true);

    page1 = bpm.FetchPage(0); EXPECT_EQ(strncmp(page1->GetContent(), "test1", 5), 0);
    page2 = bpm.FetchPage(1); EXPECT_EQ(strncmp(page2->GetContent(), "test2", 5), 0);
    page3 = bpm.FetchPage(2); EXPECT_EQ(page3, nullptr);
        
    bpm.FlushAllData();
    disk_manager.ShutDown();
    remove("test.db");
}

} // dsbus