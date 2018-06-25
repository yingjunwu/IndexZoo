#pragma once

#include "base_run.h"

template<typename KeyT>
class BaseRunMerger {

public:
  BaseRunMerger() {}

  virtual ~BaseRunMerger() {}

  virtual void find(const KeyT key, std::vector<uint64_t> &values) = 0;

  virtual void print() const = 0;

};