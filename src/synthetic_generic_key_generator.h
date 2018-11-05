#pragma once

#include "fast_random.h"

#include "base_generic_key_generator.h"

class SyntheticGenericKeyGenerator : public BaseGenericKeyGenerator {
public:

  SyntheticGenericKeyGenerator(const uint64_t thread_id) : fast_rand_(thread_id) {}
  virtual ~SyntheticGenericKeyGenerator() {}

  virtual void get_next_key(GenericKey &key) final {
    fast_rand_.next_readable_chars(key.size(), key.raw());
  }

private:
  FastRandom fast_rand_;
};
