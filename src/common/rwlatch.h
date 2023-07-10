#pragma once

#include <mutex>
#include <shared_mutex>

namespace dsbus {

/**
 * Reader-Writer latch backed by std::mutex.
 */
class ReaderWriterLatch {
 public:
  /**
   * Acquire a write latch.
   */
  void WLock() { mutex_.lock(); }

  /**
   * Release a write latch.
   */
  void WUnlock() { mutex_.unlock(); }

  /**
   * Acquire a read latch.
   */
  void RLock() { mutex_.lock_shared(); }

  /**
   * Release a read latch.
   */
  void RUnlock() { mutex_.unlock_shared(); }

 private:
  std::shared_mutex mutex_;
};

}  // namespace bustub