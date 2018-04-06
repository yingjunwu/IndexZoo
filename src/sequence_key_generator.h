#pragma once

#include <atomic>

#include "base_key_generator.h"

// generate sequence data, from 0 to infinity
template<typename KeyT>
class SequenceKeyGenerator : public BaseKeyGenerator<KeyT> {
public:

  SequenceKeyGenerator(const uint64_t thread_id) : 
    local_curr_key_(0), 
    local_max_key_(0),
    rand_gen_(thread_id) {}

  virtual ~SequenceKeyGenerator() {}
  
  virtual uint64_t get_insert_key() final {
    if (local_curr_key_ == local_max_key_){
      KeyT key = global_curr_key_.fetch_add(batch_key_count_, std::memory_order_relaxed);
      local_curr_key_ = key;
      local_max_key_ = key + batch_key_count_;
    }

    KeyT ret_key = local_curr_key_;
    ++local_curr_key_;
    return ret_key;

  }

  virtual uint64_t get_read_key() final {
    return rand_gen_.next<KeyT>() % global_curr_key_;
  }

private:
  KeyT local_curr_key_;
  KeyT local_max_key_;

  FastRandom rand_gen_;

  const KeyT batch_key_count_ = 1ull << 10;

  static std::atomic<KeyT> global_curr_key_;
  
};

template<typename KeyT>
std::atomic<KeyT> SequenceKeyGenerator<KeyT>::global_curr_key_(0);