#pragma once

#include <cmath>
#include <random>

#include "base_key_generator.h"

// generate data following the normal distribution
class Uint64NormalKeyGenerator : public BaseKeyGenerator {
public:

  Uint64NormalKeyGenerator(const uint64_t thread_id, const double upper_bound, const double stddev) :
    BaseKeyGenerator(thread_id),
    upper_bound_(upper_bound),
    dist_gen_(thread_id),
    dist_(upper_bound / 2, stddev) {}
  
  virtual ~Uint64NormalKeyGenerator() {}

  virtual uint64_t get_insert_key() final {
    return uint64_t(std::round(dist_(dist_gen_)));
  }

  virtual uint64_t get_read_key() final {
    return rand_gen_.next() % upper_bound_;
  }
  
private:
  uint64_t upper_bound_;

  std::default_random_engine dist_gen_;
  std::normal_distribution<double> dist_;
};
