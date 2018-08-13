#pragma once

#include "libcuckoo/cuckoohash_map.hh"

#include "base_dynamic_generic_index.h"

namespace dynamic_index {
namespace multithread {

template<typename ValueT>
class LibcuckooGenericIndex : public BaseDynamicGenericIndex<ValueT> {

public:
  LibcuckooGenericIndex(GenericDataTable<ValueT> *table_ptr) : BaseDynamicGenericIndex<ValueT>(table_ptr) {}
  virtual ~LibcuckooGenericIndex() {}

  virtual void insert(const GenericKey &key, const Uint64 &value) final {

    container_.upsert(key, [&value](std::vector<Uint64>& vec) { vec.push_back(value); }, 1, value);
  }

  virtual void find(const GenericKey &key, std::vector<Uint64> &values) final {
    container_.find(key, values);
  }

  virtual void find_range(const GenericKey &lhs_key, const GenericKey &rhs_key, std::vector<Uint64> &values) final {
    assert(false);
  }

  virtual void erase(const GenericKey &key) final {
    container_.erase(key);
  }

  virtual size_t size() const final {
    return container_.size();
  }

private:
  cuckoohash_map<GenericKey, std::vector<Uint64>, GenericKeyHasher> container_;
};

}
}