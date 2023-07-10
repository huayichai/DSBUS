#include <bitset>
#include <functional>
#include <numeric>
#include <random>
#include <unordered_set>
#include <thread>  // NOLINT

#include "trie/trie.hpp"
#include "gtest/gtest.h"

namespace dsbus {

// Generate n random strings
std::vector<std::string> GenerateNRandomString(int n) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<char> char_dist('A', 'z');
  std::uniform_int_distribution<int> len_dist(1, 30);

  std::unordered_set<std::string> sets;
  while (sets.size() < (size_t)n) {
    int str_len = len_dist(gen);
    std::string rand_str;
    for (int i = 0; i < str_len; ++i) {
      rand_str.push_back(char_dist(gen));
    }
    sets.insert(rand_str);
  }

  std::vector<std::string> rand_strs;
  rand_strs.reserve(n);
  for (auto &rand_str : sets) {
    rand_strs.push_back(rand_str);
  }
  return rand_strs;
}

TEST(TrieNodeTest, InsertTest) {
  // Test Insert
  //  When same key is inserted twice, insert should return nullptr
  // When inserted key and unique_ptr's key does not match, return nullptr
  auto t = TrieNode('a');
  auto child_node = t.InsertChildNode('b', std::make_unique<TrieNode>('b'));
  EXPECT_NE(child_node, nullptr);
  EXPECT_EQ((*child_node)->GetKeyChar(), 'b');

  child_node = t.InsertChildNode('b', std::make_unique<TrieNode>('b'));
  EXPECT_EQ(child_node, nullptr);

  child_node = t.InsertChildNode('d', std::make_unique<TrieNode>('b'));
  EXPECT_EQ(child_node, nullptr);

  child_node = t.InsertChildNode('c', std::make_unique<TrieNode>('c'));
  EXPECT_EQ((*child_node)->GetKeyChar(), 'c');
}

TEST(TrieNodeTest, RemoveTest) {
  auto t = TrieNode('a');
  __attribute__((unused)) auto child_node = t.InsertChildNode('b', std::make_unique<TrieNode>('b'));
  child_node = t.InsertChildNode('c', std::make_unique<TrieNode>('c'));

  t.RemoveChildNode('b');
  EXPECT_EQ(t.HasChild('b'), false);
  EXPECT_EQ(t.HasChildren(), true);
  child_node = t.GetChildNode('b');
  EXPECT_EQ(child_node, nullptr);

  t.RemoveChildNode('c');
  EXPECT_EQ(t.HasChild('c'), false);
  EXPECT_EQ(t.HasChildren(), false);
  child_node = t.GetChildNode('c');
  EXPECT_EQ(child_node, nullptr);
}

TEST(TrieTest, InsertTest) {
  {
    Trie trie;
    trie.Insert<std::string>("abc", "d");
    bool success = true;
    auto val = trie.GetValue<std::string>("abc", &success);
    EXPECT_EQ(success, true);
    EXPECT_EQ(val, "d");
  }

  // Insert empty string key should return false
  {
    Trie trie;
    auto success = trie.Insert<std::string>("", "d");
    EXPECT_EQ(success, false);
    trie.GetValue<std::string>("", &success);
    EXPECT_EQ(success, false);
  }

  // Insert duplicated key should not modify existing value
  {
    Trie trie;
    bool success = trie.Insert<int>("abc", 5);
    EXPECT_EQ(success, true);

    success = trie.Insert<int>("abc", 6);
    EXPECT_EQ(success, false);

    auto val = trie.GetValue<int>("abc", &success);
    EXPECT_EQ(success, true);
    EXPECT_EQ(val, 5);
  }

  // Insert different data types
  {
    Trie trie;
    bool success = trie.Insert<int>("a", 5);
    EXPECT_EQ(success, true);
    success = trie.Insert<std::string>("aa", "val");
    EXPECT_EQ(success, true);

    EXPECT_EQ(trie.GetValue<int>("a", &success), 5);
    EXPECT_EQ(success, true);
    EXPECT_EQ(trie.GetValue<std::string>("aa", &success), "val");
    EXPECT_EQ(success, true);

    trie.GetValue<int>("aaaa", &success);
    EXPECT_EQ(success, false);
  }
}

TEST(TrieTest, RemoveTest) {
  {
    Trie trie;
    bool success = trie.Insert<int>("a", 5);
    EXPECT_EQ(success, true);
    success = trie.Insert<int>("aa", 6);
    EXPECT_EQ(success, true);
    success = trie.Insert<int>("aaa", 7);
    EXPECT_EQ(success, true);

    success = trie.Remove("aaa");
    EXPECT_EQ(success, true);
    trie.GetValue<int>("aaa", &success);
    EXPECT_EQ(success, false);

    success = trie.Insert("aaa", 8);
    EXPECT_EQ(success, true);
    EXPECT_EQ(trie.GetValue<int>("aaa", &success), 8);
    EXPECT_EQ(success, true);

    // Remove non-existant keys should return false
    success = trie.Remove("aaaa");
    EXPECT_EQ(success, false);

    success = trie.Remove("aa");
    EXPECT_EQ(success, true);
    success = trie.Remove("a");
    EXPECT_EQ(success, true);
    success = trie.Remove("aaa");
    EXPECT_EQ(success, true);
  }
  {
    Trie trie;
    bool success = trie.Insert<int>("a", 5);
    EXPECT_EQ(success, true);
    success = trie.Insert<int>("ab", 6);
    EXPECT_EQ(success, true);
    success = trie.Insert<int>("abc", 7);
    EXPECT_EQ(success, true);

    success = trie.Remove("abc");
    EXPECT_EQ(success, true);
    trie.GetValue<int>("abc", &success);
    EXPECT_EQ(success, false);

    success = trie.Insert("abc", 8);
    EXPECT_EQ(success, true);
    EXPECT_EQ(trie.GetValue<int>("abc", &success), 8);
    EXPECT_EQ(success, true);

    // Remove non-existant keys should return false
    success = trie.Remove("abcd");
    EXPECT_EQ(success, false);

    success = trie.Remove("ab");
    EXPECT_EQ(success, true);
    success = trie.Remove("a");
    EXPECT_EQ(success, true);
    success = trie.Remove("abc");
    EXPECT_EQ(success, true);
  }
}

TEST(TrieTest, MixTest) {
  for (size_t k = 0; k < 100; ++k) {
    Trie trie;
    auto keys = GenerateNRandomString(100);
    auto values = GenerateNRandomString(100);
    size_t num = keys.size();
    bool success;
    for (size_t i = 0; i < num; ++i) {
      success = trie.Insert<std::string>(keys[i], values[i]);
      ASSERT_TRUE(success);
    }
    for (size_t i = 0; i < num; ++i) {
      auto v = trie.GetValue<std::string>(keys[i], &success);
      ASSERT_TRUE(success);
      ASSERT_EQ(v, values[i]);
    }
    for (size_t i = 0; i < num; i += 2) {
      success = trie.Remove(keys[i]);
      ASSERT_TRUE(success);
    }
    for (size_t i = 0; i < num; ++i) {
        auto v = trie.GetValue<std::string>(keys[i], &success);
        if (i % 2 == 0) {
          ASSERT_FALSE(success);
        } else {
          ASSERT_TRUE(success);
          ASSERT_EQ(v, values[i]);
        }
    }
  }
}

TEST(StarterTrieTest, DISABLED_ConcurrentTest1) {
  Trie trie;
  constexpr int num_words = 1000;
  constexpr int num_bits = 10;

  std::vector<std::thread> threads;
  threads.reserve(num_words);

  auto insert_task = [&](const std::string &key, int value) {
    bool success = trie.Insert(key, value);
    EXPECT_EQ(success, true);
  };
  for (int i = 0; i < num_words; i++) {
    std::string key = std::bitset<num_bits>(i).to_string();
    threads.emplace_back(std::thread{insert_task, key, i});
  }
  for (int i = 0; i < num_words; i++) {
    threads[i].join();
  }
  threads.clear();

  auto get_task = [&](const std::string &key, int value) {
    bool success = false;
    int tval = trie.GetValue<int>(key, &success);
    EXPECT_EQ(success, true);
    EXPECT_EQ(tval, value);
  };
  for (int i = 0; i < num_words; i++) {
    std::string key = std::bitset<num_bits>(i).to_string();
    threads.emplace_back(std::thread{get_task, key, i});
  }
  for (int i = 0; i < num_words; i++) {
    threads[i].join();
  }
  threads.clear();
}


} // dsbus