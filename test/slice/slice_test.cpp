#include <iostream>
#include "slice/slice.hpp"
#include "gtest/gtest.h"

namespace dsbus {

// bool StrComp(const void *s1, const void *s2) {
    
// }

TEST(SliceTest, ConstructorTest) {
{
    auto s1 = Slice();
    auto s2 = Slice("huayichai");
    auto s3 = Slice(std::string("huayichai"));
}
{
    auto s = Slice("huayichai", 5);
    ASSERT_TRUE(s == Slice("huayi"));
}
{
    auto s1 = Slice("huayi");
    Slice s2(s1);
    ASSERT_TRUE(s1 == s2);
}
}

TEST(SliceTest, AppendTest) {
{
    auto s1 = Slice("huayi");
    auto s2 = Slice("chai");
    auto s3 = Slice("huayichai");
    s1.Append(s2);
    ASSERT_TRUE(s1 == s3);
}
{
    auto s = Slice();
    s.Append("hua");
    s.Append("yi");
    s.Append("chai");
    ASSERT_TRUE(s == Slice("huayichai"));
}
}

TEST(SliceTest, SubSliceTest) {
{
    auto s = Slice("huayichai");
    auto first_name = s.SubSlice(0, 5);
    ASSERT_TRUE(first_name == Slice("huayi"));
    auto last_name = s.SubSlice(5, 4);
    ASSERT_TRUE(last_name == Slice("chai"));
}
{
    auto s = Slice("huayichai");
    auto empty_slice = s.SubSlice(9, 1);
    ASSERT_TRUE(empty_slice == Slice(""));
    auto last_name = s.SubSlice(5, 100);
    ASSERT_TRUE(last_name == Slice("chai"));
}
}

TEST(SliceTest, SubRangeTest) {
{
    auto s1 = Slice("huayichai");
    s1.SubRange(0, 4);
    ASSERT_TRUE(s1 == Slice("huayi"));

    auto s2 = Slice("huayichai");
    s2.SubRange(3, 4);
    ASSERT_TRUE(s2 == Slice("yi"));
}
{
    auto s1 = Slice("huayichai");
    s1.SubRange(5, -1);
    ASSERT_TRUE(s1 == Slice("chai"));

    auto s2 = Slice("huayichai");
    s2.SubRange(-4, -1);
    ASSERT_TRUE(s2 == Slice("chai"));
}
{
    auto s1 = Slice("huayichai");
    s1.SubRange(5, 1);
    ASSERT_TRUE(s1 == Slice(""));

    auto s2 = Slice("huayichai");
    s2.SubRange(100, 200);
    ASSERT_TRUE(s2 == Slice(""));

    auto s3 = Slice("huayichai");
    s3.SubRange(0, 100);
    ASSERT_TRUE(s3 == Slice("huayichai"));

    auto s4 = Slice("huayichai");
    s4.SubRange(-1, 4);
    ASSERT_TRUE(s4 == Slice(""));
}
}


} // dsbus