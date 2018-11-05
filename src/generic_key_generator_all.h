#pragma once

#include "synthetic_generic_key_generator.h"

enum class WorkloadType {
  SyntheticType = 0,
  UsernameType = 1,
  UrlType = 2,
};

static BaseGenericKeyGenerator* construct_generic_key_generator(const WorkloadType workload_type, const uint64_t thread_id) {

  if (workload_type == WorkloadType::SyntheticType) {

    return new SyntheticGenericKeyGenerator(thread_id);

  } else {
    ASSERT(false, "unsupported yet...");
    return nullptr;
  }

}
