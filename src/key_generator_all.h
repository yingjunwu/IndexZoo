#pragma once

#include "uniform_key_generator.h"
#include "normal_key_generator.h"
#include "lognormal_key_generator.h"
#include "sequence_key_generator.h"

enum class DistributionType {
  SequenceType = 0,
  UniformType,
  NormalType,
  LognormalType,
};

static const double INVALID_KEY_STDDEV = std::numeric_limits<double>::max();
static const uint64_t INVALID_KEY_BOUND = 0;
static const uint64_t DEFAULT_KEY_BOUND = std::numeric_limits<uint64_t>::max();


template<typename KeyT>
static BaseKeyGenerator<KeyT>* construct_key_generator(const DistributionType distribution_type, const uint64_t thread_id, const uint64_t key_bound, const double key_stddev) {

  if (distribution_type == DistributionType::SequenceType) {

    return new SequenceKeyGenerator<KeyT>(thread_id);

  } else if (distribution_type == DistributionType::UniformType) {

    return new UniformKeyGenerator<KeyT>(thread_id, key_bound);

  } else if (distribution_type == DistributionType::NormalType) {

    return new NormalKeyGenerator<KeyT>(thread_id, key_bound, key_stddev);
  
  } else {
    assert(distribution_type == DistributionType::LognormalType);

    return new LognormalKeyGenerator<KeyT>(thread_id, key_bound, key_stddev);
  
  }
}

static void validate_key_generator_params(const DistributionType distribution_type, const uint64_t key_bound, const double key_stddev) {

  // validate distribution parameters.
  if (distribution_type == DistributionType::SequenceType) {

    std::cout << "key generator type: sequence" << std::endl;

  } else if (distribution_type == DistributionType::UniformType) {
    
    if (key_bound == INVALID_KEY_BOUND) {
      std::cerr << "expected key generator type: uniform" << std::endl;
      std::cerr << "error: upper bound cannot be 0!" << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    std::cout << "key generator type: uniform" << std::endl;
    std::cout << "upper bound: " << key_bound << std::endl;

  } else if (distribution_type == DistributionType::NormalType) {

    if (key_bound == INVALID_KEY_BOUND) {
      std::cerr << "expected key generator type: normal" << std::endl;
      std::cerr << "error: upper bound cannot be 0!" << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    if (key_stddev == INVALID_KEY_STDDEV) {
      std::cerr << "expected key generator type: normal" << std::endl;
      std::cerr << "error: stddev unset!" << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    std::cout << "key generator type: normal" << std::endl;
    std::cout << "upper bound: " << key_bound << std::endl;
    std::cout << "stddev: " << key_stddev << std::endl;

  } else if (distribution_type == DistributionType::LognormalType) {


    if (key_bound == INVALID_KEY_BOUND) {
      std::cerr << "expected key generator type: lognormal" << std::endl;
      std::cerr << "error: upper bound cannot be 0!" << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    if (key_stddev == INVALID_KEY_STDDEV) {
      std::cerr << "expected key generator type: lognormal" << std::endl;
      std::cerr << "error: stddev unset! suggested range: [0, 0.5]." << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    std::cout << "key generator type: lognormal" << std::endl;
    std::cout << "upper bound: " << key_bound << std::endl;
    std::cout << "stddev: " << key_stddev << std::endl;

  }

}