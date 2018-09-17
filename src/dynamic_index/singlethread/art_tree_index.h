#pragma once

#include "art_tree/art.h"

#include "base_dynamic_index.h"

namespace dynamic_index {
namespace singlethread {

template<typename KeyT, typename ValueT>
class ArtTreeIndex : public BaseDynamicIndex<KeyT, ValueT> {

public:
  ArtTreeIndex(DataTable<KeyT, ValueT> *table_ptr) : BaseDynamicIndex<KeyT, ValueT>(table_ptr) {
    art_tree_init(&container_);
  }
  
  virtual ~ArtTreeIndex() {
    art_tree_destroy(&container_);
  }

  virtual void insert(const KeyT &key, const Uint64 &offset) final {
    KeyT bs_key = byte_swap<KeyT>(key);
    art_insert(&container_, (unsigned char*)(&bs_key), sizeof(KeyT), offset);
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &offsets) final {
    KeyT bs_key = byte_swap<KeyT>(key);
    art_search(&container_, (unsigned char*)(&bs_key), sizeof(KeyT), offsets);
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &offsets) final {
    KeyT bs_lhs_key = byte_swap<KeyT>(lhs_key);
    KeyT bs_rhs_key = byte_swap<KeyT>(rhs_key);
    art_range_scan(&container_, (unsigned char*)(&bs_lhs_key), sizeof(KeyT), (unsigned char*)(&bs_rhs_key), sizeof(KeyT), offsets);
  }

  virtual void scan_full(std::vector<Uint64> &offsets, const size_t count) final {
    art_scan_limit(&container_, offsets, count);
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

}
}
