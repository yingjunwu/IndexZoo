#pragma once

#include "stx_btree/btree_multimap.h"

#include "base_dynamic_generic_index.h"


namespace dynamic_index {
namespace singlethread {


class StxBtreeGenericIndex : public BaseDynamicGenericIndex {

public:
  StxBtreeGenericIndex(GenericDataTable *table_ptr) : BaseDynamicGenericIndex(table_ptr) {}
  virtual ~StxBtreeGenericIndex() {}

  virtual void insert(const GenericKey &key, const Uint64 &value) final {

    container_.insert(std::pair<GenericKey, Uint64>(key, value));
  }

  virtual void find(const GenericKey &key, std::vector<Uint64> &values) final {
    auto ret = container_.equal_range(key);
    for (auto iter = ret.first; iter != ret.second; ++iter) {
      values.push_back(iter->second);
    }
  }

  virtual void find_range(const GenericKey &lhs_key, const GenericKey &rhs_key, std::vector<Uint64> &values) final {

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

  virtual void scan(const GenericKey &key, std::vector<Uint64> &values) final {
    for (auto it = container_.begin(); it != container_.end(); ++it) {
      if (it->first == key) {
        values.push_back(it->second);
      }
      if (it->first > key) {
        return;
      }
    }
  }

  virtual void scan_reverse(const GenericKey &key, std::vector<Uint64> &values) final {}

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

  virtual void erase(const GenericKey &key) final {
    container_.erase(key);
  }

  virtual size_t size() const final {
    return container_.size();
  }

private:
  stx::btree_multimap<GenericKey, Uint64> container_;
};

}
}