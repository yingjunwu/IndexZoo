#pragma once

#include <map>
#include <unordered_map>

#include "base_dynamic_generic_index.h"

namespace dynamic_index {
namespace singlethread {

class SdTreeGenericIndex : public BaseDynamicGenericIndex {

public:
  SdTreeGenericIndex(GenericDataTable *table_ptr) : BaseDynamicGenericIndex(table_ptr) {}

  virtual ~SdTreeGenericIndex() {}
  
  virtual void insert(const GenericKey &key, const Uint64 &offset) final {}

  virtual void find(const GenericKey &key, std::vector<Uint64> &offsets) final {}

  virtual void find_range(const GenericKey &lhs_key, const GenericKey &rhs_key, std::vector<Uint64> &offsets) final {}

  virtual void scan_full(std::vector<Uint64> &offsets, const size_t count) final {
  }

  virtual void erase(const GenericKey &key) final {
  }

  virtual size_t size() const final {
    return 0;
  }

  virtual void print() const final {
  }

private:

};

}
}