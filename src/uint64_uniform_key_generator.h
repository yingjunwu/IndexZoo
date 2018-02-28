#pragma once

#include "base_key_generator.h"

class Uint64UniformKeyGenerator : public BaseKeyGenerator {
public:

  Uint64UniformKeyGenerator(const uint64_t thread_id) : BaseKeyGenerator(thread_id) {}

  virtual ~Uint64UniformKeyGenerator() {}
  
  virtual uint64_t get_insert_key() final {
    // generate sequence data
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
    // generate data following the normal distribution
    else {
      return rand_gen_.next() % global_max_key_;
    }
  }
  
};
