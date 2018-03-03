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

static BaseKeyGenerator* construct_key_generator(const DistributionType distribution_type, const uint64_t thread_id, const uint64_t key_bound = INVALID_DIST_PARAM, const double p1 = INVALID_KEY_BOUND, const double p2 = INVALID_KEY_BOUND) {

  if (distribution_type == DistributionType::SequenceType) {

    return new Uint64SequenceKeyGenerator(thread_id);

  } else if (distribution_type == DistributionType::UniformType) {

    return new Uint64UniformKeyGenerator(thread_id, key_bound);

  } else if (distribution_type == DistributionType::NormalType) {

    return new Uint64NormalKeyGenerator(thread_id, key_bound, p1);
  
  } else {
    assert(distribution_type == DistributionType::LognormalType);

    return new Uint64LognormalKeyGenerator(thread_id, key_bound, p1, p2);
  
  }
}