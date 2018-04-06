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

static const double INVALID_DIST_PARAM = std::numeric_limits<double>::max();
static const uint64_t INVALID_KEY_BOUND = std::numeric_limits<uint64_t>::max();

template<typename KeyT>
static BaseKeyGenerator<KeyT>* construct_key_generator(const DistributionType distribution_type, const uint64_t thread_id, const uint64_t key_bound, const double dist_param_1, const double dist_param_2) {

  if (distribution_type == DistributionType::SequenceType) {

    // return new SequenceKeyGenerator<KeyT>(thread_id);
    return nullptr;

  } else if (distribution_type == DistributionType::UniformType) {

    return new UniformKeyGenerator<KeyT>(thread_id, key_bound);

  } else if (distribution_type == DistributionType::NormalType) {

    return new NormalKeyGenerator<KeyT>(thread_id, key_bound, dist_param_1);
  
  } else {
    assert(distribution_type == DistributionType::LognormalType);

    return new LognormalKeyGenerator<KeyT>(thread_id, key_bound, dist_param_1);
  
  }
}

static void validate_key_generator_params(const DistributionType distribution_type, const uint64_t key_bound, const double dist_param_1, const double dist_param_2) {

  // validate distribution parameters.
  if (distribution_type == DistributionType::SequenceType) {

    std::cout << "key generator type: sequence" << std::endl;

  } else if (distribution_type == DistributionType::UniformType) {
    
    if (key_bound == INVALID_KEY_BOUND) {
      std::cerr << "expected key generator type: uniform" << std::endl;
      std::cerr << "error: upper bound unset!" << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    std::cout << "key generator type: uniform" << std::endl;
    std::cout << "upper bound: " << key_bound << std::endl;

  } else if (distribution_type == DistributionType::NormalType) {

    if (key_bound == INVALID_KEY_BOUND) {
      std::cerr << "expected key generator type: normal" << std::endl;
      std::cerr << "error: upper bound unset!" << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    if (dist_param_1 == INVALID_DIST_PARAM) {
      std::cerr << "expected key generator type: normal" << std::endl;
      std::cerr << "error: stddev (dist_param_1) unset!" << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    std::cout << "key generator type: normal" << std::endl;
    std::cout << "upper bound: " << key_bound << std::endl;
    std::cout << "stddev (dist_param_1): " << dist_param_1 << std::endl;

  } else if (distribution_type == DistributionType::LognormalType) {


    if (key_bound == INVALID_KEY_BOUND) {
      std::cerr << "expected key generator type: lognormal" << std::endl;
      std::cerr << "error: upper bound unset!" << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    if (dist_param_1 == INVALID_DIST_PARAM) {
      std::cerr << "expected key generator type: lognormal" << std::endl;
      std::cerr << "error: stddev (dist_param_1) unset! suggested range: [0, 0.5]." << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    std::cout << "key generator type: lognormal" << std::endl;
    std::cout << "upper bound: " << key_bound << std::endl;
    std::cout << "stddev (dist_param_1): " << dist_param_1 << std::endl;

  }

}