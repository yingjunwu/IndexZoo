#pragma once

#include "art_tree/art.h"

#include "base_dynamic_generic_index.h"

namespace dynamic_index {
namespace singlethread {

class ArtTreeGenericIndex : public BaseDynamicGenericIndex {

public:
  ArtTreeGenericIndex(GenericDataTable *table_ptr) : BaseDynamicGenericIndex(table_ptr) {
    art_tree_init(&container_);
  }
  
  virtual ~ArtTreeGenericIndex() {
    art_tree_destroy(&container_);
  }

  virtual void insert(const GenericKey &key, const Uint64 &offset) final {
    art_insert(&container_, (unsigned char*)(key.raw()), key.size(), offset);
  }

  virtual void find(const GenericKey &key, std::vector<Uint64> &offsets) final {
    art_search(&container_, (unsigned char*)(key.raw()), key.size(), offsets);
  }

  virtual void find_range(const GenericKey &lhs_key, const GenericKey &rhs_key, std::vector<Uint64> &offsets) final {
    art_range_scan(&container_, (unsigned char*)(lhs_key.raw()), lhs_key.size(), (unsigned char*)(rhs_key.raw()), rhs_key.size(), offsets);
  }

  virtual void scan_full(std::vector<Uint64> &offsets, const size_t count) final {
    art_scan_limit(&container_, offsets, count);
  }

  virtual void erase(const GenericKey &key) final {
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
