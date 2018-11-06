#pragma once

#include "fast_random.h"

#include "base_generic_key_generator.h"

class SyntheticGenericKeyGenerator : public BaseGenericKeyGenerator {
public:

  SyntheticGenericKeyGenerator(const uint64_t thread_id, const size_t key_size) : fast_rand_(thread_id), key_size_(key_size) {}
  virtual ~SyntheticGenericKeyGenerator() {}

  virtual void get_next_key(GenericKey &key) final {
    key.resize(key_size_);
    fast_rand_.next_readable_chars(key.size(), key.raw());
  }

private:
  FastRandom fast_rand_;
  size_t key_size_;
};
