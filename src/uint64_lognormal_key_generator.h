#pragma once

#include <cmath>

#include "base_key_generator.h"

// generate data following the lognormal distribution
class Uint64LognormalKeyGenerator : public BaseKeyGenerator {
public:

  Uint64LognormalKeyGenerator(const uint64_t thread_id, const double upper_bound, const double s) :
    BaseKeyGenerator(thread_id),
    upper_bound_(upper_bound),
    dist_gen_(thread_id),
    dist_(0, s) {}
  
  virtual ~Uint64LognormalKeyGenerator() {}

  virtual uint64_t get_insert_key() final {
    return uint64_t(dist_(dist_gen_) * upper_bound_ / 10);
  }

  virtual uint64_t get_read_key() final {
    return rand_gen_.next<uint64_t>() % upper_bound_;
  }
  
private:
  uint64_t upper_bound_;

  std::default_random_engine dist_gen_;
  std::lognormal_distribution<double> dist_;
};
