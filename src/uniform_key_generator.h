#pragma once

#include "fast_random.h"

#include "base_key_generator.h"

// generate data following the uniform distribution
template<typename KeyT>
class UniformKeyGenerator : public BaseKeyGenerator<KeyT> {
public:

  UniformKeyGenerator(const uint64_t thread_id, const uint64_t upper_bound) : 
    upper_bound_(upper_bound), 
    rand_gen_(thread_id) {}

  virtual ~UniformKeyGenerator() {}
  
  virtual KeyT get_insert_key() final {
    return rand_gen_.next<KeyT>() % upper_bound_;
  }

  virtual KeyT get_read_key() final {
    return rand_gen_.next<KeyT>() % upper_bound_;
  }

private:
  KeyT upper_bound_;

  FastRandom rand_gen_;
};
