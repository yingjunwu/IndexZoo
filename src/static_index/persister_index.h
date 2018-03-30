#pragma once

#include <vector>

#include "base_static_index.h"

// for data persisting only

namespace static_index {

template<typename KeyT, typename ValueT>
class PersisterIndex : public BaseStaticIndex<KeyT, ValueT> {

public:
  PersisterIndex(DataTable<KeyT, ValueT> *table_ptr) : BaseStaticIndex<KeyT, ValueT>(table_ptr) {}

  virtual ~PersisterIndex() {}

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {}

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) final {}

  virtual void reorganize() final { this->base_reorganize(); }

  virtual void print() const final {}

  virtual void print_stats() const final {}

  void persist_keys(const std::string &filename) const {

    std::ofstream persist_file;
    persist_file.open(filename);
    for (size_t i = 0; i < this->size_; ++i) {
      persist_file << this->container_[i].key_ << std::endl;
    }
    persist_file.close();
  }

};

}