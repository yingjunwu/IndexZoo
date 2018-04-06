#pragma once

#include <cmath>
#include <random>

#include "fast_random.h"

#include "base_key_generator.h"

// generate data following the normal distribution
template<typename KeyT>
class NormalKeyGenerator : public BaseKeyGenerator<KeyT> {
public:

  NormalKeyGenerator(const uint64_t thread_id, const uint64_t upper_bound, const double stddev) :
    upper_bound_(upper_bound),
    rand_gen_(thread_id), 
    dist_gen_(thread_id),
    dist_(upper_bound / 2, stddev) {}
  
  virtual ~NormalKeyGenerator() {}

  virtual KeyT get_insert_key() final {
    return KeyT(std::round(dist_(dist_gen_)));
  }

  virtual KeyT get_read_key() final {
    return rand_gen_.next<KeyT>() % upper_bound_;
  }
  
private:
  KeyT upper_bound_;

  FastRandom rand_gen_;

  std::default_random_engine dist_gen_;
  std::normal_distribution<double> dist_;
};
