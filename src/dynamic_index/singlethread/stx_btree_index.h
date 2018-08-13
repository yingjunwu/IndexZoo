#pragma once

#include "stx_btree/btree_multimap.h"

#include "base_dynamic_index.h"


namespace dynamic_index {
namespace singlethread {

template<typename KeyT, typename ValueT>
class StxBtreeIndex : public BaseDynamicIndex<KeyT, ValueT> {

public:
  StxBtreeIndex(DataTable<KeyT, ValueT> *table_ptr) : BaseDynamicIndex<KeyT, ValueT>(table_ptr) {}
  virtual ~StxBtreeIndex() {}

  virtual void insert(const KeyT &key, const Uint64 &value) final {

    container_.insert(std::pair<KeyT, Uint64>(key, value));
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {
    auto ret = container_.equal_range(key);
    for (auto iter = ret.first; iter != ret.second; ++iter) {
      values.push_back(iter->second);
    }
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) final {
    
    if (lhs_key > rhs_key) { return; }

    if (lhs_key == rhs_key) { 
      find(lhs_key, values);
      return;
    }

    auto itlow = container_.lower_bound(lhs_key);
    auto itup = container_.upper_bound(rhs_key);

    for (auto it = itlow; it != itup; ++it) {
      values.push_back(it->second);
    }
  }

  virtual void scan(const KeyT &key, std::vector<Uint64> &values) final {
    for (auto it = container_.begin(); it != container_.end(); ++it) {
      if (it->first == key) {
        values.push_back(it->second);
      }
      if (it->first > key) {
        return;
      }
    }
  }

  virtual void scan_reverse(const KeyT &key, std::vector<Uint64> &values) final {}

  virtual void scan_full(std::vector<Uint64> &values, const size_t count) final {
    size_t i = 0;
    for (auto it = container_.begin(); it != container_.end(); ++it) {
      if (i < count) {
        values.push_back(it->second);
        ++i;
      } else {
        return;
      }
    }
  }

  virtual void erase(const KeyT &key) final {
    container_.erase(key);
  }

  virtual size_t size() const final {
    return container_.size();
  }

private:
  stx::btree_multimap<KeyT, Uint64> container_;
};

}
}