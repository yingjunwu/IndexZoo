#pragma once

#include "base_dynamic_index.h"

namespace dynamic_index {
namespace singlethread {

template<typename KeyT, typename ValueT>
class SkiplistIndex : public BaseDynamicIndex<KeyT, ValueT> {

public:
  SkiplistIndex(DataTable<KeyT, ValueT> *table_ptr) : BaseDynamicIndex<KeyT, ValueT>(table_ptr) {
  }
  
  virtual ~SkiplistIndex() {
  }

  virtual void insert(const KeyT &key, const Uint64 &offset) final {
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &offsets) final {    
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &offsets) final {
  }

  virtual void erase(const KeyT &key) final {
  }

  virtual size_t size() const final {
    return 0;
  }

};

}
}
