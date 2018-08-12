#pragma once

#include "base_generic_index.h"

template<typename ValueT>
class BaseDynamicGenericIndex : public BaseGenericIndex<ValueT> {

public:
  BaseDynamicGenericIndex(GenericDataTable<ValueT> *table_ptr) : 
    BaseGenericIndex<ValueT>(table_ptr) {}

  virtual ~BaseDynamicGenericIndex() {}

  virtual void scan(const char *key, const uint64_t key_size, std::vector<Uint64> &values) override {}

  virtual void scan_reverse(const char *key, const uint64_t key_size, std::vector<Uint64> &values) override {}

  virtual void scan_full(std::vector<Uint64> &values, const size_t count) override {}

  virtual void prepare_threads(const size_t thread_count) override {}

  virtual void register_thread(const size_t thread_id) override {}

  virtual void reorganize() final {}

  virtual void print() const override {}

};
