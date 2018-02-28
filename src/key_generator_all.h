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

