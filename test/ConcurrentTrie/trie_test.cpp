#include <fmt/format.h>
#include <numeric>

#include "ConcurrentTrie/trie.h"
#include "gtest/gtest.h"

namespace dsbus {

TEST(TrieTest, ConstructorTest) { auto trie = Trie(); }

TEST(TrieTest, BasicPutTest) {
    auto trie = Trie();
    trie = trie.Put<uint32_t>("test-int", 233);
    trie = trie.Put<uint64_t>("test-int2", 23333333);
    trie = trie.Put<std::string>("test-string", "test");
    trie = trie.Put<std::string>("", "empty-key");
}

TEST(TrieTest, PutGetOnePath) {
    auto trie = Trie();
    trie = trie.Put<uint32_t>("111", 111);
    trie = trie.Put<uint32_t>("11", 11);
    trie = trie.Put<uint32_t>("1111", 1111);
    trie = trie.Put<uint32_t>("11", 22);
    ASSERT_EQ(*trie.Get<uint32_t>("11"), 22);
    ASSERT_EQ(*trie.Get<uint32_t>("111"), 111);
    ASSERT_EQ(*trie.Get<uint32_t>("1111"), 1111);
}

TEST(TrieTest, BasicRemoveTest1) {
  auto trie = Trie();
  // Put something
  trie = trie.Put<uint32_t>("test", 2333);
  ASSERT_EQ(*trie.Get<uint32_t>("test"), 2333);
  trie = trie.Put<uint32_t>("te", 23);
  ASSERT_EQ(*trie.Get<uint32_t>("te"), 23);
  trie = trie.Put<uint32_t>("tes", 233);
  ASSERT_EQ(*trie.Get<uint32_t>("tes"), 233);
  // Delete something
  trie = trie.Remove("test");
  trie = trie.Remove("tes");
  trie = trie.Remove("te");

  ASSERT_EQ(trie.Get<uint32_t>("te"), nullptr);
  ASSERT_EQ(trie.Get<uint32_t>("tes"), nullptr);
  ASSERT_EQ(trie.Get<uint32_t>("test"), nullptr);
}

TEST(TrieTest, BasicRemoveTest2) {
  auto trie = Trie();
  // Put something
  trie = trie.Put<uint32_t>("test", 2333);
  ASSERT_EQ(*trie.Get<uint32_t>("test"), 2333);
  trie = trie.Put<uint32_t>("te", 23);
  ASSERT_EQ(*trie.Get<uint32_t>("te"), 23);
  trie = trie.Put<uint32_t>("tes", 233);
  ASSERT_EQ(*trie.Get<uint32_t>("tes"), 233);
  // Delete something
  trie = trie.Remove("te");
  trie = trie.Remove("tes");
  trie = trie.Remove("test");

  ASSERT_EQ(trie.Get<uint32_t>("te"), nullptr);
  ASSERT_EQ(trie.Get<uint32_t>("tes"), nullptr);
  ASSERT_EQ(trie.Get<uint32_t>("test"), nullptr);
}

TEST(TrieTest, MismatchTypeTest) {
  auto trie = Trie();
  // Put something
  trie = trie.Put<uint32_t>("test", 2333);
  ASSERT_EQ(trie.Get<std::string>("test"), nullptr);
}

TEST(TrieTest, CopyOnWriteTest1) {
  auto empty_trie = Trie();
  // Put something
  auto trie1 = empty_trie.Put<uint32_t>("test", 2333);
  auto trie2 = trie1.Put<uint32_t>("te", 23);
  auto trie3 = trie2.Put<uint32_t>("tes", 233);

  // Delete something
  auto trie4 = trie3.Remove("te");
  auto trie5 = trie3.Remove("tes");
  auto trie6 = trie3.Remove("test");

  // Check each snapshot
  ASSERT_EQ(*trie3.Get<uint32_t>("te"), 23);
  ASSERT_EQ(*trie3.Get<uint32_t>("tes"), 233);
  ASSERT_EQ(*trie3.Get<uint32_t>("test"), 2333);

  ASSERT_EQ(trie4.Get<uint32_t>("te"), nullptr);
  ASSERT_EQ(*trie4.Get<uint32_t>("tes"), 233);
  ASSERT_EQ(*trie4.Get<uint32_t>("test"), 2333);

  ASSERT_EQ(*trie5.Get<uint32_t>("te"), 23);
  ASSERT_EQ(trie5.Get<uint32_t>("tes"), nullptr);
  ASSERT_EQ(*trie5.Get<uint32_t>("test"), 2333);

  ASSERT_EQ(*trie6.Get<uint32_t>("te"), 23);
  ASSERT_EQ(*trie6.Get<uint32_t>("tes"), 233);
  ASSERT_EQ(trie6.Get<uint32_t>("test"), nullptr);
}

TEST(TrieTest, CopyOnWriteTest2) {
  auto empty_trie = Trie();
  // Put something
  auto trie1 = empty_trie.Put<uint32_t>("test", 2333);
  auto trie2 = trie1.Put<uint32_t>("te", 23);
  auto trie3 = trie2.Put<uint32_t>("tes", 233);

  // Override something
  auto trie4 = trie3.Put<std::string>("te", "23");
  auto trie5 = trie3.Put<std::string>("tes", "233");
  auto trie6 = trie3.Put<std::string>("test", "2333");

  // Check each snapshot
  ASSERT_EQ(*trie3.Get<uint32_t>("te"), 23);
  ASSERT_EQ(*trie3.Get<uint32_t>("tes"), 233);
  ASSERT_EQ(*trie3.Get<uint32_t>("test"), 2333);

  ASSERT_EQ(*trie4.Get<std::string>("te"), "23");
  ASSERT_EQ(*trie4.Get<uint32_t>("tes"), 233);
  ASSERT_EQ(*trie4.Get<uint32_t>("test"), 2333);

  ASSERT_EQ(*trie5.Get<uint32_t>("te"), 23);
  ASSERT_EQ(*trie5.Get<std::string>("tes"), "233");
  ASSERT_EQ(*trie5.Get<uint32_t>("test"), 2333);

  ASSERT_EQ(*trie6.Get<uint32_t>("te"), 23);
  ASSERT_EQ(*trie6.Get<uint32_t>("tes"), 233);
  ASSERT_EQ(*trie6.Get<std::string>("test"), "2333");
}

TEST(TrieTest, CopyOnWriteTest3) {
  auto empty_trie = Trie();
  // Put something
  auto trie1 = empty_trie.Put<uint32_t>("test", 2333);
  auto trie2 = trie1.Put<uint32_t>("te", 23);
  auto trie3 = trie2.Put<uint32_t>("", 233);

  // Override something
  auto trie4 = trie3.Put<std::string>("te", "23");
  auto trie5 = trie3.Put<std::string>("", "233");
  auto trie6 = trie3.Put<std::string>("test", "2333");

  // Check each snapshot
  ASSERT_EQ(*trie3.Get<uint32_t>("te"), 23);
  ASSERT_EQ(*trie3.Get<uint32_t>(""), 233);
  ASSERT_EQ(*trie3.Get<uint32_t>("test"), 2333);

  ASSERT_EQ(*trie4.Get<std::string>("te"), "23");
  ASSERT_EQ(*trie4.Get<uint32_t>(""), 233);
  ASSERT_EQ(*trie4.Get<uint32_t>("test"), 2333);

  ASSERT_EQ(*trie5.Get<uint32_t>("te"), 23);
  ASSERT_EQ(*trie5.Get<std::string>(""), "233");
  ASSERT_EQ(*trie5.Get<uint32_t>("test"), 2333);

  ASSERT_EQ(*trie6.Get<uint32_t>("te"), 23);
  ASSERT_EQ(*trie6.Get<uint32_t>(""), 233);
  ASSERT_EQ(*trie6.Get<std::string>("test"), "2333");
}

TEST(TrieTest, MixedTest) {
  auto trie = Trie();
  for (uint32_t i = 0; i < 23333; i++) {
    std::string key = fmt::format("{:#05}", i);
    std::string value = fmt::format("value-{:#08}", i);
    trie = trie.Put<std::string>(key, value);
  }
  auto trie_full = trie;
  for (uint32_t i = 0; i < 23333; i += 2) {
    std::string key = fmt::format("{:#05}", i);
    std::string value = fmt::format("new-value-{:#08}", i);
    trie = trie.Put<std::string>(key, value);
  }
  auto trie_override = trie;
  for (uint32_t i = 0; i < 23333; i += 3) {
    std::string key = fmt::format("{:#05}", i);
    trie = trie.Remove(key);
  }
  auto trie_final = trie;

  // verify trie_full
  for (uint32_t i = 0; i < 23333; i++) {
    std::string key = fmt::format("{:#05}", i);
    std::string value = fmt::format("value-{:#08}", i);
    ASSERT_EQ(*trie_full.Get<std::string>(key), value);
  }

  // verify trie_override
  for (uint32_t i = 0; i < 23333; i++) {
    std::string key = fmt::format("{:#05}", i);
    if (i % 2 == 0) {
      std::string value = fmt::format("new-value-{:#08}", i);
      ASSERT_EQ(*trie_override.Get<std::string>(key), value);
    } else {
      std::string value = fmt::format("value-{:#08}", i);
      ASSERT_EQ(*trie_override.Get<std::string>(key), value);
    }
  }

  // verify final trie
  for (uint32_t i = 0; i < 23333; i++) {
    std::string key = fmt::format("{:#05}", i);
    if (i % 3 == 0) {
      ASSERT_EQ(trie_final.Get<std::string>(key), nullptr);
    } else if (i % 2 == 0) {
      std::string value = fmt::format("new-value-{:#08}", i);
      ASSERT_EQ(*trie_final.Get<std::string>(key), value);
    } else {
      std::string value = fmt::format("value-{:#08}", i);
      ASSERT_EQ(*trie_final.Get<std::string>(key), value);
    }
  }
}

TEST(TrieTest, PointerStability) {
  auto trie = Trie();
  trie = trie.Put<uint32_t>("test", 2333);
  auto *ptr_before = trie.Get<std::string>("test");
  trie = trie.Put<uint32_t>("tes", 233);
  trie = trie.Put<uint32_t>("te", 23);
  auto *ptr_after = trie.Get<std::string>("test");
  ASSERT_EQ(reinterpret_cast<uint64_t>(ptr_before), reinterpret_cast<uint64_t>(ptr_after));
}

TEST(TrieTest, NonCopyableTest) {
  using Integer = std::unique_ptr<uint32_t>;
  auto trie = Trie();
  trie = trie.Put<Integer>("tes", std::make_unique<uint32_t>(233));
  trie = trie.Put<Integer>("te", std::make_unique<uint32_t>(23));
  trie = trie.Put<Integer>("test", std::make_unique<uint32_t>(2333));
  ASSERT_EQ(**trie.Get<Integer>("te"), 23);
  ASSERT_EQ(**trie.Get<Integer>("tes"), 233);
  ASSERT_EQ(**trie.Get<Integer>("test"), 2333);
  trie = trie.Remove("te");
  trie = trie.Remove("tes");
  trie = trie.Remove("test");
  ASSERT_EQ(trie.Get<Integer>("te"), nullptr);
  ASSERT_EQ(trie.Get<Integer>("tes"), nullptr);
  ASSERT_EQ(trie.Get<Integer>("test"), nullptr);
}

} // dsbus