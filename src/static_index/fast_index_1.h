#pragma once

#include <vector>

#include "base_static_index.h"

namespace static_index {

template<typename KeyT, typename ValueT>
class FastIndex : public BaseStaticIndex<KeyT, ValueT> {

  const size_t SIMD_SIZE = 16; // unit: byte (128 bits)
  const size_t CACHELINE_SIZE = 64; // unit: byte
  const size_t PAGE_SIZE = 4096; // unit: byte (4 KB)

  const size_t SIMD_BLOCK_CAPACITY = 16 / sizeof(KeyT)

public:
  FastIndex(DataTable<KeyT, ValueT> *table_ptr) : BaseStaticIndex<KeyT, ValueT>(table_ptr) {}

  virtual ~FastIndex() {}

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {

    if (this->size_ == 0) {
      return;
    }

    if (key > key_max_ || key < key_min_) {
      return;
    }
    if (key_max_ == key_min_) {
      if (key_max_ == key) {
        for (size_t i = 0; i < this->size_; ++i) {
          values.push_back(this->container_[i].value_);
        }
      }
      return;
    }

    return;
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) final {
    assert(lhs_key < rhs_key);

    if (this->size_ == 0) {
      return;
    }
    if (lhs_key > key_max_ || rhs_key < key_min_) {
      return;
    }

  }

  virtual void reorganize() final {

    this->base_reorganize();

    size_t inner_node_size = std::pow(2.0, num_layers_) - 1;


    key_min_ = this->container_[0].key_;
    key_max_ = this->container_[this->size_ - 1].key_;
  }



  virtual void print() const final {

  }

  virtual void print_stats() const final {

  }

private:

  void construct_inner_layers() {
    ASSERT(num_layers_ != 0, "number of layers cannot be 0");

  }

  void construct_inner_layers_internal() {

  }

private:
  KeyT key_min_;
  KeyT key_max_;
};

}