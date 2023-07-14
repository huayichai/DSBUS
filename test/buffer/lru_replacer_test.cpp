#include <iostream>
#include "buffer/lru_replacer.hpp"
#include "gtest/gtest.h"

namespace dsbus {

TEST(LRUReplacerTest, VictimTest) {
    int v;
    {
        LRUReplacer lru(3);
        EXPECT_EQ(true, lru.Victim(&v));
        EXPECT_EQ(0, v);
        EXPECT_EQ(true, lru.Victim(&v));
        EXPECT_EQ(1, v);
        EXPECT_EQ(true, lru.Victim(&v));
        EXPECT_EQ(2, v);
        EXPECT_EQ(false, lru.Victim(&v));
    }
}

TEST(LRUReplacerTest, PinTest) {
    int v;
    {
        LRUReplacer lru(3);
        lru.Pin(0);
        lru.Victim(&v);
        lru.Victim(&v);
        EXPECT_EQ(false, lru.Victim(&v));
    }
    {
        LRUReplacer lru(3);
        lru.Pin(0);
        lru.Pin(1);
        lru.Pin(2);
        EXPECT_EQ(false, lru.Victim(&v));
    }
    {
        LRUReplacer lru(3);
        lru.Pin(2);
        lru.Pin(1);
        EXPECT_EQ(true, lru.Victim(&v));
        EXPECT_EQ(0, v);
    }
}

TEST(LRUReplacerTest, UnpinTest) {
    int v;
    {
        LRUReplacer lru(3);
        lru.Unpin(0);
        lru.Unpin(1);
        lru.Unpin(2);
        
        lru.Victim(&v);
        EXPECT_EQ(0, v);
    }
    {
        LRUReplacer lru(3);
        lru.Pin(0);
        lru.Pin(0);
        lru.Pin(1);
        lru.Pin(2);

        lru.Unpin(0);
        lru.Unpin(1);
        lru.Unpin(2);

        lru.Victim(&v);
        EXPECT_EQ(1, v);
        lru.Victim(&v);
        EXPECT_EQ(2, v);

        lru.Unpin(0);
        lru.Unpin(1);
        lru.Unpin(2);

        lru.Victim(&v);
        EXPECT_EQ(0, v);
    }
}

TEST(LRUReplacerTest, SampleTest1) {
    int v;
    LRUReplacer lru(3);
    
    lru.Victim(&v); // 0
    lru.Victim(&v); // 1
    lru.Victim(&v); // 2

    lru.Unpin(1);
    lru.Unpin(2);

    lru.Victim(&v);
    EXPECT_EQ(1, v);

    lru.Unpin(0);
    lru.Pin(2);

    lru.Victim(&v);
    EXPECT_EQ(0, v);
}

TEST(LRUReplacerTest, SampleTest2) {
    int v;
    LRUReplacer lru(7);
    // 0 1 2 3 4 5 6
    EXPECT_EQ(7, lru.Size());
    lru.Pin(0);
    lru.Pin(1);
    // 2 3 4 5 6
    lru.Victim(&v); EXPECT_EQ(2, v);
    // 3 4 5 6
    lru.Pin(5);
    // 3 4 6
    lru.Victim(&v); EXPECT_EQ(3, v);
    // 4 6
    lru.Unpin(1);
    lru.Unpin(2);
    // 4 6 1 2
    EXPECT_EQ(4, lru.Size());
    lru.Victim(&v); EXPECT_EQ(4, v);
    lru.Victim(&v); EXPECT_EQ(6, v);
    // 1 2
    EXPECT_EQ(2, lru.Size());
    lru.Pin(2);
    lru.Pin(1);
    // 
    EXPECT_EQ(0, lru.Size());
    EXPECT_EQ(false, lru.Victim(&v));
    lru.Unpin(4);
    lru.Unpin(1);
    lru.Unpin(0);
    // 4 1 0
    lru.Pin(1);
    // 4 0
    lru.Victim(&v); EXPECT_EQ(4, v);
    lru.Victim(&v); EXPECT_EQ(0, v);
    //
    lru.Unpin(0);
    lru.Unpin(1);
    lru.Unpin(2);
    lru.Unpin(3);
    lru.Unpin(4);
    lru.Unpin(5);
    lru.Unpin(6);
    EXPECT_EQ(7, lru.Size());
}

} // dsbus