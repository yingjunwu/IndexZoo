#pragma once

#include "base_key_generator.h"

// generate data following the uniform distribution
class Uint64UniformKeyGenerator : public BaseKeyGenerator {
public:

  Uint64UniformKeyGenerator(const uint64_t thread_id, const double upper_bound) : 
    BaseKeyGenerator(thread_id), 
    upper_bound_(upper_bound) {}

  virtual ~Uint64UniformKeyGenerator() {}
  
  virtual uint64_t get_insert_key() final {
    return rand_gen_.next() % upper_bound_;
  }

  virtual uint64_t get_read_key() final {
    return rand_gen_.next() % upper_bound_;
  }

private:
  uint64_t upper_bound_;

};
