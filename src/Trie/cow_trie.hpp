#pragma once

#include <algorithm>
#include <assert.h>
#include <cstddef>
#include <future>  // NOLINT
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace dsbus {

// A TrieNode is a node in a Trie.
class COWTrieNode {
 public:
  // Create a TrieNode with no children.
  COWTrieNode() = default;

  // Create a TrieNode with some children.
  explicit COWTrieNode(std::map<char, std::shared_ptr<const COWTrieNode>> children) : children_(std::move(children)) {}

  virtual ~COWTrieNode() = default;

  // Clone returns a copy of this TrieNode. If the TrieNode has a value, the value is copied. The return
  // type of this function is a unique_ptr to a TrieNode.
  //
  // You cannot use the copy constructor to clone the node because it doesn't know whether a `TrieNode`
  // contains a value or not.
  //
  // Note: if you want to convert `unique_ptr` into `shared_ptr`, you can use `std::shared_ptr<T>(std::move(ptr))`.
  virtual auto Clone() const -> std::unique_ptr<COWTrieNode> { return std::make_unique<COWTrieNode>(children_); }

  // A map of children, where the key is the next character in the key, and the value is the next TrieNode.
  std::map<char, std::shared_ptr<const COWTrieNode>> children_;

  // Indicates if the node is the terminal node.
  bool is_value_node_{false};

  // You can add additional fields and methods here. But in general, you don't need to add extra fields to
  // complete this project.
};

// A TrieNodeWithValue is a TrieNode that also has a value of type T associated with it.
template <class T>
class COWTrieNodeWithValue : public COWTrieNode {
 public:
  // Create a trie node with no children and a value.
  explicit COWTrieNodeWithValue(std::shared_ptr<T> value) : value_(std::move(value)) { this->is_value_node_ = true; }

  // Create a trie node with children and a value.
  COWTrieNodeWithValue(std::map<char, std::shared_ptr<const COWTrieNode>> children, std::shared_ptr<T> value)
      : COWTrieNode(std::move(children)), value_(std::move(value)) {
    this->is_value_node_ = true;
  }

  // Override the Clone method to also clone the value.
  //
  // Note: if you want to convert `unique_ptr` into `shared_ptr`, you can use `std::shared_ptr<T>(std::move(ptr))`.
  auto Clone() const -> std::unique_ptr<COWTrieNode> override {
    return std::make_unique<COWTrieNodeWithValue<T>>(children_, value_);
  }

  // The value associated with this trie node.
  std::shared_ptr<T> value_;
};

// A Trie is a data structure that maps strings to values of type T. All operations on a Trie should not
// modify the trie itself. It should reuse the existing nodes as much as possible, and create new nodes to
// represent the new trie.
class COWTrie {
 public:
  // Create an empty trie.
  COWTrie() : root_(std::make_shared<const COWTrieNode>()) {}

  // Get the value associated with the given key.
  // 1. If the key is not in the trie, return nullptr.
  // 2. If the key is in the trie but the type is mismatched, return nullptr.
  // 3. Otherwise, return the value.
  template <class T>
  auto Get(std::string_view key) const -> const T * {
    auto node = Find(key);
    if (node == nullptr) {
      return nullptr;
    }
    if (!node->is_value_node_) {
      return nullptr;
    }
    auto value_node = dynamic_cast<const COWTrieNodeWithValue<T> *>(node.get());
    if (value_node == nullptr) {
      return nullptr;
    }
    // return dynamic_cast<const T *>(value_node->value_.get());
    auto res = value_node->value_.get();
    return res;
  }

  // Put a new key-value pair into the trie. If the key already exists, overwrite the value.
  // Returns the new trie.
  template <class T>
  auto Put(std::string_view key, T value) const -> COWTrie {
    size_t key_len = key.size();
    size_t i = 0;
    auto cur = std::shared_ptr<COWTrieNode>(std::move(root_->Clone()));
    if (key_len == 0) {
      cur = std::make_shared<COWTrieNodeWithValue<T>>(cur->children_, std::make_shared<T>(std::move(value)));
    }
    auto new_root = cur;
    auto prev = cur;
    while (i < key_len) {
      auto it = cur->children_.find(key[i]);
      if (it != cur->children_.end()) {
        prev = cur;
        cur = std::shared_ptr<COWTrieNode>(std::move(it->second->Clone()));
        if (i == key_len - 1) {
          auto value_node = std::make_shared<COWTrieNodeWithValue<T>>(cur->children_, std::make_shared<T>(std::move(value)));
          cur = std::shared_ptr<COWTrieNode>(value_node);
        }
      } else {
        prev = cur;
        if (i != key_len - 1) {
          cur = std::make_shared<COWTrieNode>();
        } else {
          auto value_node = std::make_shared<COWTrieNodeWithValue<T>>(std::make_shared<T>(std::move(value)));
          cur = std::shared_ptr<COWTrieNode>(value_node);
        }
      }
      prev->children_[key[i]] = std::shared_ptr<const COWTrieNode>(cur);
      ++i;
    }
    return COWTrie(std::shared_ptr<const COWTrieNode>(new_root));
  }

  // Remove the key from the trie. If the key does not exist, return the original trie.
  // Otherwise, returns the new trie.
  auto Remove(std::string_view key) const -> COWTrie {
    size_t key_len = key.size();
    size_t i = 0;
    auto cur = std::shared_ptr<COWTrieNode>(std::move(root_->Clone()));
    auto new_root = cur;
    auto prev = cur;
    while (i < key_len) {
      auto it = cur->children_.find(key[i]);
      if (it == cur->children_.end()) {
        return *this;
      }
      prev = cur;
      cur = std::shared_ptr<COWTrieNode>(std::move(it->second->Clone()));
      prev->children_[key[i]] = std::shared_ptr<const COWTrieNode>(cur);
      ++i;
    }
    if (!cur->is_value_node_) {
      return *this;
    }
    if (cur->children_.empty()) {
      auto it = prev->children_.find(key[key_len - 1]);
      assert(it != prev->children_.end());
      prev->children_.erase(it);
    } else {
      auto clone_node = std::shared_ptr<COWTrieNode>(std::move(cur->Clone()));
      auto node = std::make_shared<COWTrieNode>(clone_node->children_);
      prev->children_[key[key_len - 1]] = std::shared_ptr<const COWTrieNode>(node);
    }
    return COWTrie(std::shared_ptr<const COWTrieNode>(new_root));
  }

 private:
  // The root of the trie.
  std::shared_ptr<const COWTrieNode> root_{nullptr};

  // Create a new trie with the given root.
  explicit COWTrie(std::shared_ptr<const COWTrieNode> root) : root_(std::move(root)) {}

  auto Find(std::string_view key) const -> std::shared_ptr<const COWTrieNode> {
    size_t key_len = key.size();
    size_t idx = 0;
    auto node = root_;
    while (idx < key_len) {
      auto it = node->children_.find(key[idx++]);
      if (it == node->children_.end()) {
        return nullptr;
      }
      node = it->second;
    }
    return node;
  }
};


// This class is used to guard the value returned by the trie. It holds a reference to the root so
// that the reference to the value will not be invalidated.
template <class T>
class ValueGuard {
 public:
  ValueGuard(COWTrie root, const T &value) : root_(std::move(root)), value_(value) {}
  auto operator*() const -> const T & { return value_; }

 private:
  COWTrie root_;
  const T &value_;
};

// This class is a thread-safe wrapper around the COWTrie class. It provides a simple interface for
// accessing the trie. It should allow concurrent reads and a single write operation at the same
// time.
class COWTrieStore {
 public:
  // This function returns a ValueGuard object that holds a reference to the value in the trie. If
  // the key does not exist in the trie, it will return std::nullopt.
  template <class T>
  auto Get(std::string_view key) -> std::optional<ValueGuard<T>> {
    root_lock_.lock();
    COWTrie trie = root_;
    root_lock_.unlock();
    auto res = trie.Get<T>(key);
    if (res == nullptr) {
      return std::nullopt;
    }
    return {ValueGuard<T>(trie, *res)};
  }

  // This function will insert the key-value pair into the trie. If the key already exists in the
  // trie, it will overwrite the value.
  template <class T>
  void Put(std::string_view key, T value) {
    write_lock_.lock();
    auto trie = root_.Put(key, std::move(value));

    root_lock_.lock();
    root_ = trie;
    root_lock_.unlock(); 

    write_lock_.unlock();
  }

  // This function will remove the key-value pair from the trie.
  void Remove(std::string_view key) {
    write_lock_.lock();
    COWTrie trie = root_;  
    trie = trie.Remove(key);

    root_lock_.lock();
    root_ = trie;
    root_lock_.unlock(); 

    write_lock_.unlock();
  }

 private:
  // This mutex protects the root. Every time you want to access the trie root or modify it, you
  // will need to take this lock.
  std::mutex root_lock_;

  // This mutex sequences all writes operations and allows only one write operation at a time.
  std::mutex write_lock_;

  // Stores the current root for the trie.
  COWTrie root_;
};


}  // namespace bustub