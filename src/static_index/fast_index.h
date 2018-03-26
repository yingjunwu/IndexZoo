#pragma once

#include <vector>

#include "base_static_index.h"

namespace static_index {

template<typename KeyT, typename ValueT>
class FastIndex : public BaseStaticIndex<KeyT, ValueT> {

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

    size_t offset_find = this->size_;
    size_t offset_begin = 0;
    size_t offset_end = this->size_ - 1;
    while (true) {
      size_t offset_lookup = (offset_begin + offset_end) / 2;
      KeyT key_lookup = this->container_[offset_lookup].key_;
      if (key > key_lookup) {
        offset_begin = offset_lookup + 1;
        if (offset_begin > offset_end) {
          return;
        } else {
          continue; 
        }
      } else if (key < key_lookup) {
        offset_end = offset_lookup - 1;
        if (offset_begin > offset_end) {
          return;
        } else {
          continue;
        }
      } else {
        offset_find = offset_lookup;
        break;
      }

    }

    ASSERT(offset_find < this->size_ && offset_find >= 0, "invalid offset: " << offset_find << " " << this->size_);

    values.push_back(this->container_[offset_find].value_);

    // move left
    int offset_find_lhs = offset_find - 1;
    while (offset_find_lhs >= 0) {

      if (this->container_[offset_find_lhs].key_ == key) {
        values.push_back(this->container_[offset_find_lhs].value_);
        offset_find_lhs -= 1;
      } else {
        break;
      }
    }
    // move right
    int offset_find_rhs = offset_find + 1;
    while (offset_find_rhs < this->size_ - 1) {

      if (this->container_[offset_find_rhs].key_ == key) {
        values.push_back(this->container_[offset_find_rhs].value_);
        offset_find_rhs += 1;
      } else {
        break;
      }
    }
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
    key_min_ = this->container_[0].key_;
    key_max_ = this->container_[this->size_ - 1].key_;
  }

  virtual void print() const final {

  }

  virtual void print_stats() const final {

  }

private:
  KeyT key_min_;
  KeyT key_max_;
};

}