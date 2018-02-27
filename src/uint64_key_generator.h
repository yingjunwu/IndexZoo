#pragma once

#include <atomic>
#include <random>

#include "fast_random.h"

class Uint64KeyGenerator {
public:
  Uint64KeyGenerator(const uint64_t thread_id) :
    rand_gen_(thread_id),
    thread_id_(thread_id), 
    local_curr_key_(0), 
    local_max_key_(0) {}
  
  uint64_t get_insert_key() {
    // sequence data
    if (global_max_key_ == 0) {

      if (local_curr_key_ == local_max_key_){
        uint64_t key = global_curr_key_.fetch_add(batch_key_count_, std::memory_order_relaxed);
        local_curr_key_ = key;
        local_max_key_ = key + batch_key_count_;
      }

      uint64_t ret_key = local_curr_key_;
      ++local_curr_key_;
      return ret_key;

    } 
    // data that follows certain distribution
    else {
      return rand_gen_.next() % global_max_key_;

    }
  }

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
  
private:
  FastRandom rand_gen_;

  // std::default_random_engine generator_;
  // std::uniform_int_distribution<int> uniform_dist_;
  // std::normal_distribution<int> normal_dist_;

  uint64_t thread_id_;
  
  uint64_t local_curr_key_;
  uint64_t local_max_key_;
  const uint64_t batch_key_count_ = 1ull << 10;

  static std::atomic<uint64_t> global_curr_key_;
  static uint64_t global_max_key_;
};
