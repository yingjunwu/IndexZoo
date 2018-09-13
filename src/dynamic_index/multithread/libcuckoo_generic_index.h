#pragma once

#include "libcuckoo/cuckoohash_map.hh"

#include "base_dynamic_generic_index.h"

namespace dynamic_index {
namespace multithread {

class LibcuckooGenericIndex : public BaseDynamicGenericIndex {

public:
  LibcuckooGenericIndex(GenericDataTable *table_ptr) : BaseDynamicGenericIndex(table_ptr) {}
  virtual ~LibcuckooGenericIndex() {}

  virtual void insert(const GenericKey &key, const Uint64 &offset) final {

    container_.upsert(key, [&offset](std::vector<Uint64>& vec) { vec.push_back(offset); }, 1, offset);
  }

  virtual void find(const GenericKey &key, std::vector<Uint64> &offsets) final {
    container_.find(key, offsets);
  }

  virtual void find_range(const GenericKey &lhs_key, const GenericKey &rhs_key, std::vector<Uint64> &offsets) final {
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