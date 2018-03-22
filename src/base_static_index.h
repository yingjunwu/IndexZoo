#pragma once

#include <iostream>
#include <cassert>
#include <cstdint>
#include <vector>

#include "offset.h"
#include "data_table.h"

template<typename KeyT, typename ValueT>
class BaseStaticIndex {

protected:
  struct KeyValuePair {
    KeyValuePair() : key_(0), value_(0) {}
    KeyValuePair(const KeyT key, const Uint64 value) : key_(key), value_(value) {}

    KeyT key_;
    Uint64 value_;
  };

  static bool compare_func(KeyValuePair &lhs, KeyValuePair &rhs) {
    return lhs.key_ < rhs.key_;
  }

public:
  BaseStaticIndex(DataTable<KeyT, ValueT> *table_ptr) : 
    table_ptr_(table_ptr), container_(nullptr), size_(0) {}
  
  virtual ~BaseStaticIndex() {
    delete[] container_;
    container_ = nullptr;
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) = 0;

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) = 0;


  void scan(const KeyT &key, std::vector<Uint64> &values) final {
    for (size_t i = 0; i < this->size_; ++i) {
      if (this->container_[i].key_ == key) {
        values.push_back(this->container_[i].value_);
      }
    }
  }

  void scan_reverse(const KeyT &key, std::vector<Uint64> &values) final {
    for (int i = this->size_ - 1; i >= 0; --i) {
      if (this->container_[i].key_ == key) {
        values.push_back(this->container_[i].value_);
      }
    }
  }

  virtual void reorganize() = 0;

  virtual void print() const = 0;

  virtual void print_stats() const {}

  size_t size() const { return size_; }

protected:
  void base_reorganize() {

    assert(container_ == nullptr && size_ == 0);

    size_t capacity = 0;
    capacity = table_ptr_->size();
    
    container_ = new KeyValuePair[capacity];

    DataTableIterator<KeyT, ValueT> iterator(table_ptr_);
    while (iterator.has_next()) {
      auto entry = iterator.next();
      container_[size_].key_ = *(entry.key_);
      container_[size_].value_ = entry.offset_;
      ++size_;
    }

    std::sort(container_, container_ + size_, compare_func);

  }

protected:

  DataTable<KeyT, ValueT> *table_ptr_;

  KeyValuePair *container_;
  size_t size_;

};
