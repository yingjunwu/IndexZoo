#pragma once

#include "uint64_uniform_key_generator.h"
#include "uint64_normal_key_generator.h"
#include "uint64_lognormal_key_generator.h"
#include "uint64_sequence_key_generator.h"


enum class DistributionType {
  SequenceType = 0,
  UniformType,
  NormalType,
  LognormalType,
};

static const double INVALID_DIST_PARAM = std::numeric_limits<double>::max();
static const uint64_t INVALID_KEY_BOUND = std::numeric_limits<uint64_t>::max();

static BaseKeyGenerator* construct_key_generator(const DistributionType distribution_type, const uint64_t thread_id, const uint64_t key_bound, const double dist_param_1, const double dist_param_2) {

  if (distribution_type == DistributionType::SequenceType) {

    return new Uint64SequenceKeyGenerator(thread_id);

  } else if (distribution_type == DistributionType::UniformType) {

    return new Uint64UniformKeyGenerator(thread_id, key_bound);

  } else if (distribution_type == DistributionType::NormalType) {

    return new Uint64NormalKeyGenerator(thread_id, key_bound, dist_param_1);
  
  } else {
    assert(distribution_type == DistributionType::LognormalType);

    return new Uint64LognormalKeyGenerator(thread_id, key_bound, dist_param_1);
  
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