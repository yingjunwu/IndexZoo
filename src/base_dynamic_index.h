#pragma once

#include "base_index.h"

template<typename KeyT, typename ValueT>
class BaseDynamicIndex : public BaseIndex<KeyT, ValueT> {

public:
  BaseDynamicIndex(DataTable<KeyT, ValueT> *table_ptr) : 
    BaseIndex<KeyT, ValueT>(table_ptr) {}

  virtual ~BaseDynamicIndex() {}

  virtual void scan(const KeyT &key, std::vector<Uint64> &values) final {}

  virtual void scan_reverse(const KeyT &key, std::vector<Uint64> &values) final {}

  virtual void prepare_threads(const size_t thread_count) override {}

  virtual void register_thread(const size_t thread_id) override {}

  virtual void reorganize() override {}

  virtual void print() const override {}

  virtual void print_stats() const override {}

};
