#pragma once

#include "libcuckoo/cuckoohash_map.hh"

#include "base_index.h"


namespace dynamic_index {
namespace multithread {

template<typename KeyT>
class LibcuckooIndex : public BaseIndex<KeyT> {

public:
  LibcuckooIndex(const size_t size_hint) : container_(size_hint) {}
  LibcuckooIndex() {}
  virtual ~LibcuckooIndex() {}

  virtual void insert(const KeyT &key, const Uint64 &value) final {

    container_.upsert(key, [&value](std::vector<Uint64>& vec) { vec.push_back(value); }, 1, value);
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {    
    container_.find(key, values);
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) final {
    assert(false);
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
