#pragma once

#include <atomic>

#include "base_key_generator.h"

// generate sequence data, from 0 to infinity
class Uint64SequenceKeyGenerator : public BaseKeyGenerator {
public:

  Uint64SequenceKeyGenerator(const uint64_t thread_id) : 
    BaseKeyGenerator(thread_id), 
    local_curr_key_(0), 
    local_max_key_(0) {}

  virtual ~Uint64SequenceKeyGenerator() {}
  
  virtual uint64_t get_insert_key() final {
    if (local_curr_key_ == local_max_key_){
      uint64_t key = global_curr_key_.fetch_add(batch_key_count_, std::memory_order_relaxed);
      local_curr_key_ = key;
      local_max_key_ = key + batch_key_count_;
    }

    uint64_t ret_key = local_curr_key_;
    ++local_curr_key_;
    return ret_key;

  }

  virtual uint64_t get_read_key() final {
    return rand_gen_.next<uint64_t>() % global_curr_key_;
  }

private:
  uint64_t local_curr_key_;
  uint64_t local_max_key_;

  const uint64_t batch_key_count_ = 1ull << 10;

  static std::atomic<uint64_t> global_curr_key_;
  
};

