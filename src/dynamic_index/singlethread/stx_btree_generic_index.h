#pragma once

#include "stx_btree/btree_multimap.h"

#include "base_dynamic_generic_index.h"


namespace dynamic_index {
namespace singlethread {


class StxBtreeGenericIndex : public BaseDynamicGenericIndex {

public:
  StxBtreeGenericIndex(GenericDataTable *table_ptr) : BaseDynamicGenericIndex(table_ptr) {}
  virtual ~StxBtreeGenericIndex() {}

  virtual void insert(const GenericKey &key, const Uint64 &offset) final {

    container_.insert(std::pair<GenericKey, Uint64>(key, offset));
  }

  virtual void find(const GenericKey &key, std::vector<Uint64> &offsets) final {
    auto ret = container_.equal_range(key);
    for (auto iter = ret.first; iter != ret.second; ++iter) {
      offsets.push_back(iter->second);
    }
  }

  virtual void find_range(const GenericKey &lhs_key, const GenericKey &rhs_key, std::vector<Uint64> &offsets) final {

    if (lhs_key > rhs_key) { return; }

    if (lhs_key == rhs_key) { 
      find(lhs_key, offsets);
      return;
    }

    auto itlow = container_.lower_bound(lhs_key);
    auto itup = container_.upper_bound(rhs_key);

    for (auto it = itlow; it != itup; ++it) {
      offsets.push_back(it->second);
    }
  }

  virtual void scan(const GenericKey &key, std::vector<Uint64> &offsets) final {
    for (auto it = container_.begin(); it != container_.end(); ++it) {
      if (it->first == key) {
        offsets.push_back(it->second);
      }
      if (it->first > key) {
        return;
      }
    }
  }

  virtual void scan_reverse(const GenericKey &key, std::vector<Uint64> &offsets) final {}

  virtual void scan_full(std::vector<Uint64> &offsets, const size_t count) final {
    size_t i = 0;
    for (auto it = container_.begin(); it != container_.end(); ++it) {
      if (i < count) {
        offsets.push_back(it->second);
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