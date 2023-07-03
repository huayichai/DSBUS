#include "trie.h"
#include <assert.h>
#include <string_view>

namespace dsbus {

auto Trie::Find(std::string_view key) const -> std::shared_ptr<const TrieNode> {
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

template <class T>
auto Trie::Get(std::string_view key) const -> const T * {
  auto node = Find(key);
  if (node == nullptr) {
    return nullptr;
  }
  if (!node->is_value_node_) {
    return nullptr;
  }
  auto value_node = dynamic_cast<const TrieNodeWithValue<T> *>(node.get());
  if (value_node == nullptr) {
    return nullptr;
  }
  // return dynamic_cast<const T *>(value_node->value_.get());
  auto res = value_node->value_.get();
  return res;
}


// Note that `T` might be a non-copyable type. Always use `std::move` when creating `shared_ptr` on that value.
// You should walk through the trie and create new nodes if necessary. If the node corresponding to the key already
// exists, you should create a new `TrieNodeWithValue`.
template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  size_t key_len = key.size();
  size_t i = 0;
  auto cur = std::shared_ptr<TrieNode>(std::move(root_->Clone()));
  auto new_root = cur;
  auto prev = cur;
  while (i < key_len) {
    auto it = cur->children_.find(key[i]);
    if (it != cur->children_.end()) {
      prev = cur;
      cur = std::shared_ptr<TrieNode>(std::move(it->second->Clone()));
      if (i == key_len - 1) {
        auto value_node = std::make_shared<TrieNodeWithValue<T>>(cur->children_, std::make_shared<T>(std::move(value)));
        cur = std::shared_ptr<TrieNode>(value_node);
      }
    } else {
      prev = cur;
      if (i != key_len - 1) {
        cur = std::make_shared<TrieNode>();
      } else {
        auto value_node = std::make_shared<TrieNodeWithValue<T>>(std::make_shared<T>(std::move(value)));
        cur = std::shared_ptr<TrieNode>(value_node);
      }
    }
    prev->children_[key[i]] = std::shared_ptr<const TrieNode>(cur);
    ++i;
  }
  return Trie(std::shared_ptr<const TrieNode>(new_root));
}

// You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value any more,
// you should convert it to `TrieNode`. If a node doesn't have children any more, you should remove it.
auto Trie::Remove(std::string_view key) const -> Trie {
  size_t key_len = key.size();
  size_t i = 0;
  auto cur = std::shared_ptr<TrieNode>(std::move(root_->Clone()));
  auto new_root = cur;
  auto prev = cur;
  while (i < key_len) {
    auto it = cur->children_.find(key[i]);
    if (it == cur->children_.end()) {
      return *this;
    }
    prev = cur;
    cur = std::shared_ptr<TrieNode>(std::move(it->second->Clone()));
    prev->children_[key[i]] = std::shared_ptr<const TrieNode>(cur);
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
    auto clone_node = std::shared_ptr<TrieNode>(std::move(cur->Clone()));
    auto node = std::make_shared<TrieNode>(clone_node->children_);
    prev->children_[key[key_len - 1]] = std::shared_ptr<const TrieNode>(node);
  }
  return Trie(std::shared_ptr<const TrieNode>(new_root));
}

// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and functions in the header file. However, we
// separate the implementation into a .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate them here, so that they can be picked up
// by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub