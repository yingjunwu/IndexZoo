#pragma once

#include <random>

#include "base_key_generator.h"

class Uint64NormalKeyGenerator : public BaseKeyGenerator {
public:

  Uint64NormalKeyGenerator(const uint64_t thread_id, const double p1, const double p2) :
    BaseKeyGenerator(thread_id),
    dist_gen_(thread_id),
    dist_(p1, p2) {}
  
  virtual ~Uint64NormalKeyGenerator() {}

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
      return dist_(dist_gen_) % global_max_key_;
    }
  }
  
private:
  std::default_random_engine dist_gen_;

  std::uniform_int_distribution<uint64_t> dist_;

};
