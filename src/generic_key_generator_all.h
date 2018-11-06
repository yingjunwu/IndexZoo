#pragma once

#include "synthetic_generic_key_generator.h"

enum class WorkloadType {
  SyntheticType = 0,
  UsernameType = 1,
  UrlType = 2,
};

static BaseGenericKeyGenerator* construct_generic_key_generator(const WorkloadType workload_type, const uint64_t thread_id, const size_t key_size) {

  if (workload_type == WorkloadType::SyntheticType) {

    ASSERT(key_size >= 8, "key size must be larger than 8");

    return new SyntheticGenericKeyGenerator(thread_id, key_size);

  } else {
    ASSERT(false, "unsupported yet...");
    return nullptr;
  }

}
