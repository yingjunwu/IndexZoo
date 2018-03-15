#pragma once

#include <iostream>
#include <cassert>
#include <cstdint>
#include <vector>

#include "offset.h"

template<typename KeyT>
class BaseStaticIndex {

public:
  BaseStaticIndex() {}
  virtual ~BaseStaticIndex() {}

  virtual void find(const KeyT &key, std::vector<Uint64> &values) = 0;

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) = 0;

  virtual void scan(const KeyT &key, std::vector<Uint64> &values) {}

  virtual void scan_reverse(const KeyT &key, std::vector<Uint64> &values) {}

  virtual size_t size() const = 0;
  
  virtual void reorganize() {}

  virtual void print() const {}

  virtual void print_stats() const {}
};
