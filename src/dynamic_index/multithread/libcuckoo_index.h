#pragma once

#include "libcuckoo/cuckoohash_map.hh"

#include "base_dynamic_index.h"


namespace dynamic_index {
namespace multithread {

template<typename KeyT, typename ValueT>
class LibcuckooIndex : public BaseDynamicIndex<KeyT, ValueT> {

public:
  LibcuckooIndex(DataTable<KeyT, ValueT> *table_ptr) : BaseDynamicIndex<KeyT, ValueT>(table_ptr) {}
  virtual ~LibcuckooIndex() {}

  virtual void insert(const KeyT &key, const Uint64 &offset) final {

    container_.upsert(key, [&offset](std::vector<Uint64>& vec) { vec.push_back(offset); }, 1, offset);
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &offsets) final {
    container_.find(key, offsets);
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &offsets) final {
    ASSERT(false, "hash table does not support range query");
  }

  virtual void erase(const KeyT &key) final {
    container_.erase(key);
  }

  virtual size_t size() const final {
    return container_.size();
  }

private:
  cuckoohash_map<KeyT, std::vector<Uint64>> container_;
};

}
}
