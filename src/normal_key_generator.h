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
    dist_gen_(thread_id),
    dist_(upper_bound / 2, stddev) {}
  
  virtual ~NormalKeyGenerator() {}

  virtual KeyT get_next_key() final {
    return KeyT(std::round(dist_(dist_gen_)));
  }

private:

  std::default_random_engine dist_gen_;
  std::normal_distribution<double> dist_;
};
