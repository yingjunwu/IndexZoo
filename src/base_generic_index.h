#pragma once

#include <iostream>
#include <cassert>
#include <cstdint>
#include <vector>

#include "generic_key.h"
#include "generic_data_table.h"
#include "offset.h"

template<typename ValueT>
class BaseGenericIndex {

public:
  BaseGenericIndex(GenericDataTable<ValueT> *table_ptr) :
    table_ptr_(table_ptr) {}
    
  virtual ~BaseGenericIndex() {}

  virtual void insert(const char *key, const uint64_t key_size, const Uint64 &value) = 0;

  virtual void find(const char *key, const uint64_t key_size, std::vector<Uint64> &values) = 0;

  virtual void find_range(const char *lhs_key, const uint64_t lhs_key_size, const char *rhs_key, const uint64_t rhs_key_size, std::vector<Uint64> &values) = 0;

  virtual void scan(const char *key, const uint64_t key_size, std::vector<Uint64> &values) = 0;

  virtual void scan_reverse(const char *key, const uint64_t key_size, std::vector<Uint64> &values) = 0;

  virtual void scan_full(std::vector<Uint64> &values, const size_t count = std::numeric_limits<std::size_t>::max()) = 0;

  virtual void erase(const char *key, const uint64_t key_size) = 0;

  virtual size_t size() const = 0;

  virtual void reorganize() = 0;
  
  virtual void prepare_threads(const size_t thread_count) = 0;

  virtual void register_thread(const size_t thread_id) = 0;

  virtual void print() const = 0;

protected:

  GenericDataTable<ValueT> *table_ptr_;

};
