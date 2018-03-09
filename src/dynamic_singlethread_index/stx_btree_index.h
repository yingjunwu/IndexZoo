#pragma once

#include "stx_btree/btree_multimap.h"

#include "base_index.h"


namespace dynamic_index {
namespace singlethread {

template<typename KeyT>
class StxBtreeIndex : public BaseIndex<KeyT> {

public:
  StxBtreeIndex() {}
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
    assert(lhs_key < rhs_key);

    auto itlow = container_.lower_bound(lhs_key);
    auto itup = container_.upper_bound(rhs_key + 1);

    for (auto it = itlow; it != itup; ++it) {
      values.push_back(it->second);
    }
  }

  virtual void erase(const KeyT &key) final {
    // container_.erase(key);
  }

  virtual size_t size() const final {
    return container_.size();
  }

private:
  stx::btree_multimap<KeyT, Uint64> container_;
};

}
}