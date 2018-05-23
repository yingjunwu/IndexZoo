#pragma once

#include <cmath>
#include <random>

#include "fast_random.h"

#include "base_key_generator.h"

// generate data following the lognormal distribution
template<typename KeyT>
class LognormalKeyGenerator : public BaseKeyGenerator<KeyT> {
public:

  LognormalKeyGenerator(const uint64_t thread_id, const uint64_t upper_bound, const double s) :
    upper_bound_(upper_bound),
    dist_gen_(thread_id),
    dist_(0, s) {}
  
  virtual ~LognormalKeyGenerator() {}

  virtual KeyT get_next_key() final {
    return KeyT(dist_(dist_gen_) * upper_bound_ / 10);
  }

private:

  KeyT upper_bound_;
  std::default_random_engine dist_gen_;
  std::lognormal_distribution<double> dist_;
};
