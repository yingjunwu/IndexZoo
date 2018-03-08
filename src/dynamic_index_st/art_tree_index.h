#pragma once

#include "art_tree/art.h"

#include "base_index.h"

template<typename KeyT>
class ArtTreeSTIndex : public BaseIndex<KeyT> {

public:
  ArtTreeSTIndex(const size_t size_hint) {
    art_tree_init(&container_);
  }

  ArtTreeSTIndex() {
    art_tree_init(&container_);
  }
  
  virtual ~ArtTreeSTIndex() {
    art_tree_destroy(&container_);
  }

  virtual void insert(const KeyT &key, const Uint64 &value) final {
    art_insert(&container_, (unsigned char*)(&key), sizeof(KeyT), (void*)((std::uintptr_t)value));
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {    
    Uint64 data = (std::uintptr_t)art_search(&container_, (unsigned char*)(&key), sizeof(KeyT));
    values.push_back(data);
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) final {
    // assert(false);
  }

  virtual void erase(const KeyT &key) final {
    // container_.erase(key);
  }

  virtual size_t size() const final {

    return art_size(&container_);
  }

private:
  art_tree container_;
};
