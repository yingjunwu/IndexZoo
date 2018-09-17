#pragma once

#include <iostream>
#include <cassert>
#include <cstdint>
#include <vector>

#include "data_table.h"
#include "offset.h"

template<typename KeyT, typename ValueT>
class BaseIndex {

public:
  BaseIndex(DataTable<KeyT, ValueT> *table_ptr) :
    table_ptr_(table_ptr) {}
    
  virtual ~BaseIndex() {}

  virtual void insert(const KeyT &key, const Uint64 &offset) = 0;

  virtual void find(const KeyT &key, std::vector<Uint64> &offsets) = 0;

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &offsets) = 0;

  virtual void scan(const KeyT &key, std::vector<Uint64> &offsets) = 0;

  virtual void scan_reverse(const KeyT &key, std::vector<Uint64> &offsets) = 0;

  virtual void scan_full(std::vector<Uint64> &offsets, const size_t count = std::numeric_limits<std::size_t>::max()) = 0;

  virtual void erase(const KeyT &key) = 0;

  virtual size_t size() const = 0;

  virtual void reorganize() = 0;
  
  virtual void prepare_threads(const size_t thread_count) = 0;

  virtual void register_thread(const size_t thread_id) = 0;

  virtual void print() const = 0;

protected:

  DataTable<KeyT, ValueT> *table_ptr_;

};
