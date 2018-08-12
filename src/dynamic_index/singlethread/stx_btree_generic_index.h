#pragma once

#include "stx_btree/btree_multimap.h"

#include "base_dynamic_generic_index.h"


namespace dynamic_index {
namespace singlethread {

template<typename ValueT>
class StxBtreeGenericIndex : public BaseDynamicGenericIndex<ValueT> {

public:
  StxBtreeGenericIndex(GenericDataTable<ValueT> *table_ptr) : BaseDynamicGenericIndex<ValueT>(table_ptr) {}
  virtual ~StxBtreeGenericIndex() {}

  virtual void insert(const char *key, const uint64_t key_size, const Uint64 &value) final {

    container_.insert(std::pair<GenericKey, Uint64>(GenericKey(key, key_size), value));
  }

  virtual void find(const char *key, const uint64_t key_size, std::vector<Uint64> &values) final {
    GenericKey generic_key(key, key_size);
    auto ret = container_.equal_range(generic_key);
    for (auto iter = ret.first; iter != ret.second; ++iter) {
      values.push_back(iter->second);
    }
  }

  virtual void find_range(const char *lhs_key, const uint64_t lhs_key_size, const char *rhs_key, const uint64_t rhs_key_size, std::vector<Uint64> &values) final {
    
    // GenericKey lhs_generic_key(lhs_key, lhs_key_size);
    // GenericKey rhs_generic_key(rhs_key, rhs_key_size);

    // if (lhs_generic_key > rhs_generic_key) { return; }

    // if (lhs_generic_key == rhs_generic_key) { 
    //   find(lhs_key, lhs_key_size, values);
    //   return;
    // }

    // auto itlow = container_.lower_bound(lhs_generic_key);
    // auto itup = container_.upper_bound(rhs_generic_key);

    // for (auto it = itlow; it != itup; ++it) {
    //   values.push_back(it->second);
    // }
  }

  virtual void scan(const char *key, const uint64_t key_size, std::vector<Uint64> &values) final {
    // for (auto it = container_.begin(); it != container_.end(); ++it) {
    //   if (it->first == key) {
    //     values.push_back(it->second);
    //   }
    //   if (it->first > key) {
    //     return;
    //   }
    // }
  }

  virtual void scan_reverse(const char *key, const uint64_t key_size, std::vector<Uint64> &values) final {

  }

  virtual void scan_full(std::vector<Uint64> &values, const size_t count) final {
    // size_t i = 0;
    // for (auto it = container_.begin(); it != container_.end(); ++it) {
    //   if (i < count) {
    //     values.push_back(it->second);
    //     ++i;
    //   } else {
    //     return;
    //   }
    // }
  }

  virtual void erase(const char *key, const uint64_t key_size) final {
    // container_.erase(key);
  }

  virtual size_t size() const final {
    return container_.size();
  }

private:
  stx::btree_multimap<GenericKey, Uint64> container_;
};

}
}