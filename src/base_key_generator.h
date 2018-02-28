#pragma once

#include "fast_random.h"

class BaseKeyGenerator {
public:
  
  BaseKeyGenerator(const uint64_t thread_id) :
    rand_gen_(thread_id),
    thread_id_(thread_id) {}
  
  virtual ~BaseKeyGenerator() {}

  virtual uint64_t get_insert_key() = 0;

  virtual uint64_t get_read_key() = 0;
 
protected:
  FastRandom rand_gen_;

  uint64_t thread_id_;
  
};
