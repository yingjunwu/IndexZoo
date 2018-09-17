#pragma once

#include <iostream>
#include <cassert>
#include <cstdint>
#include <vector>

#include "generic_key.h"
#include "generic_data_table.h"
#include "offset.h"

class BaseGenericIndex {

public:
  BaseGenericIndex(GenericDataTable *table_ptr) :
    table_ptr_(table_ptr) {}
    
  virtual ~BaseGenericIndex() {}

  virtual void insert(const GenericKey &key, const Uint64 &offset) = 0;

  virtual void find(const GenericKey &key, std::vector<Uint64> &offsets) = 0;

  virtual void find_range(const GenericKey &lhs_key, const GenericKey &rhs_key, std::vector<Uint64> &offsets) = 0;

  virtual void scan(const GenericKey &key, std::vector<Uint64> &offsets) = 0;

  virtual void scan_reverse(const GenericKey &key, std::vector<Uint64> &offsets) = 0;

  virtual void scan_full(std::vector<Uint64> &offsets, const size_t count = std::numeric_limits<std::size_t>::max()) = 0;

  virtual void erase(const GenericKey &key) = 0;

  virtual size_t size() const = 0;

  virtual void reorganize() = 0;
  
  virtual void prepare_threads(const size_t thread_count) = 0;

  virtual void register_thread(const size_t thread_id) = 0;

  virtual void print() const = 0;

protected:

  GenericDataTable *table_ptr_;

};
