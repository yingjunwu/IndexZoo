#pragma once

#include <atomic>

#include "fast_random.h"

class BaseKeyGenerator {
public:
  
  BaseKeyGenerator(const uint64_t thread_id) :
    rand_gen_(thread_id),
    thread_id_(thread_id), 
    local_curr_key_(0), 
    local_max_key_(0) {}
  
  virtual ~BaseKeyGenerator() {}

  virtual uint64_t get_insert_key() = 0;

  uint64_t get_read_key() {
    if (global_max_key_ == 0) {
      return rand_gen_.next() % global_curr_key_;
    } else {
      return rand_gen_.next() % global_max_key_;
    }
  }

  static void set_max_key(const uint64_t max_key) {
    global_max_key_ = max_key;
  }
  
protected:
  FastRandom rand_gen_;

  uint64_t thread_id_;
  
  uint64_t local_curr_key_;
  uint64_t local_max_key_;

  const uint64_t batch_key_count_ = 1ull << 10;

  static std::atomic<uint64_t> global_curr_key_;
  static uint64_t global_max_key_;
};
