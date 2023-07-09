#include <fmt/format.h>
#include <numeric>

#include "Trie/cow_trie.hpp"
#include "gtest/gtest.h"

namespace dsbus {

// ========== TEST for COWTrie ==========

TEST(COWTrieTest, ConstructorTest) { auto trie = COWTrie(); }

TEST(COWTrieTest, BasicPutTest) {
    auto trie = COWTrie();
    trie = trie.Put<uint32_t>("test-int", 233);
    trie = trie.Put<uint64_t>("test-int2", 23333333);
    trie = trie.Put<std::string>("test-string", "test");
    trie = trie.Put<std::string>("", "empty-key");
}

TEST(COWTrieTest, PutGetOnePath) {
    auto trie = COWTrie();
    trie = trie.Put<uint32_t>("111", 111);
    trie = trie.Put<uint32_t>("11", 11);
    trie = trie.Put<uint32_t>("1111", 1111);
    trie = trie.Put<uint32_t>("11", 22);
    ASSERT_EQ(*trie.Get<uint32_t>("11"), 22);
    ASSERT_EQ(*trie.Get<uint32_t>("111"), 111);
    ASSERT_EQ(*trie.Get<uint32_t>("1111"), 1111);
}

TEST(COWTrieTest, BasicRemoveTest1) {
  auto trie = COWTrie();
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

TEST(COWTrieTest, BasicRemoveTest2) {
  auto trie = COWTrie();
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

TEST(COWTrieTest, MismatchTypeTest) {
  auto trie = COWTrie();
  // Put something
  trie = trie.Put<uint32_t>("test", 2333);
  ASSERT_EQ(trie.Get<std::string>("test"), nullptr);
}

TEST(COWTrieTest, CopyOnWriteTest1) {
  auto empty_trie = COWTrie();
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

TEST(COWTrieTest, CopyOnWriteTest2) {
  auto empty_trie = COWTrie();
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

TEST(COWTrieTest, CopyOnWriteTest3) {
  auto empty_trie = COWTrie();
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

TEST(COWTrieTest, MixedTest) {
  auto trie = COWTrie();
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

TEST(COWTrieTest, PointerStability) {
  auto trie = COWTrie();
  trie = trie.Put<uint32_t>("test", 2333);
  auto *ptr_before = trie.Get<std::string>("test");
  trie = trie.Put<uint32_t>("tes", 233);
  trie = trie.Put<uint32_t>("te", 23);
  auto *ptr_after = trie.Get<std::string>("test");
  ASSERT_EQ(reinterpret_cast<uint64_t>(ptr_before), reinterpret_cast<uint64_t>(ptr_after));
}

TEST(COWTrieTest, NonCopyableTest) {
  using Integer = std::unique_ptr<uint32_t>;
  auto trie = COWTrie();
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

// ========== TEST for COWTrie ==========

TEST(COWTrieStoreTest, BasicTest) {
  auto store = COWTrieStore();
  ASSERT_EQ(store.Get<uint32_t>("233"), std::nullopt);
  store.Put<uint32_t>("233", 2333);
  {
    auto guard = store.Get<uint32_t>("233");
    ASSERT_EQ(**guard, 2333);
  }
  store.Remove("233");
  {
    auto guard = store.Get<uint32_t>("233");
    ASSERT_EQ(guard, std::nullopt);
  }
}

TEST(COWTrieStoreTest, GuardTest) {
  auto store = COWTrieStore();
  ASSERT_EQ(store.Get<uint32_t>("233"), std::nullopt);

  store.Put<std::string>("233", "2333");
  auto guard = store.Get<std::string>("233");
  ASSERT_EQ(**guard, "2333");

  store.Remove("233");
  {
    auto guard = store.Get<std::string>("233");
    ASSERT_EQ(guard, std::nullopt);
  }

  ASSERT_EQ(**guard, "2333");
}

TEST(COWTrieStoreTest, MixedTest) {
  auto store = COWTrieStore();
  for (uint32_t i = 0; i < 23333; i++) {
    std::string key = fmt::format("{:#05}", i);
    std::string value = fmt::format("value-{:#08}", i);
    store.Put<std::string>(key, value);
  }
  for (uint32_t i = 0; i < 23333; i += 2) {
    std::string key = fmt::format("{:#05}", i);
    std::string value = fmt::format("new-value-{:#08}", i);
    store.Put<std::string>(key, value);
  }
  for (uint32_t i = 0; i < 23333; i += 3) {
    std::string key = fmt::format("{:#05}", i);
    store.Remove(key);
  }

  // verify final trie
  for (uint32_t i = 0; i < 23333; i++) {
    std::string key = fmt::format("{:#05}", i);
    if (i % 3 == 0) {
      ASSERT_EQ(store.Get<std::string>(key), std::nullopt);
    } else if (i % 2 == 0) {
      std::string value = fmt::format("new-value-{:#08}", i);
      auto guard = store.Get<std::string>(key);
      ASSERT_EQ(**guard, value);
    } else {
      std::string value = fmt::format("value-{:#08}", i);
      auto guard = store.Get<std::string>(key);
      ASSERT_EQ(**guard, value);
    }
  }
}

TEST(COWTrieStoreTest, MixedConcurrentTest) {
  auto store = COWTrieStore();

  std::vector<std::thread> threads;

  const int keys_per_thread = 10000;

  for (int tid = 0; tid < 4; tid++) {
    std::thread t([&store, tid] {
      for (uint32_t i = 0; i < keys_per_thread; i++) {
        std::string key = fmt::format("{:#05}", i * 4 + tid);
        std::string value = fmt::format("value-{:#08}", i * 4 + tid);
        store.Put<std::string>(key, value);
      }
      for (uint32_t i = 0; i < keys_per_thread; i++) {
        std::string key = fmt::format("{:#05}", i * 4 + tid);
        store.Remove(key);
      }
      for (uint32_t i = 0; i < keys_per_thread; i++) {
        std::string key = fmt::format("{:#05}", i * 4 + tid);
        std::string value = fmt::format("new-value-{:#08}", i * 4 + tid);
        store.Put<std::string>(key, value);
      }
    });
    threads.push_back(std::move(t));
  }

  for (auto &t : threads) {
    t.join();
  }

  // verify final trie
  for (uint32_t i = 0; i < keys_per_thread * 4; i++) {
    std::string key = fmt::format("{:#05}", i);
    std::string value = fmt::format("new-value-{:#08}", i);
    auto guard = store.Get<std::string>(key);
    ASSERT_EQ(**guard, value);
  }
}




} // dsbus
