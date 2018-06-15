#pragma once

#include "base_index.h"

template<typename KeyT, typename ValueT>
class BaseStaticIndex : public BaseIndex<KeyT, ValueT> {

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
    BaseIndex<KeyT, ValueT>(table_ptr), container_(nullptr), size_(0) {}
  
  virtual ~BaseStaticIndex() {
    delete[] container_;
    container_ = nullptr;
  }

  virtual void insert(const KeyT &key, const Uint64 &value) final {}
  
  virtual void erase(const KeyT &key) final {}

  virtual void scan(const KeyT &key, std::vector<Uint64> &values) final {
    for (size_t i = 0; i < this->size_; ++i) {
      if (this->container_[i].key_ == key) {
        values.push_back(this->container_[i].value_);
      }
    }
  }

  virtual void scan_reverse(const KeyT &key, std::vector<Uint64> &values) final {
    for (int i = this->size_ - 1; i >= 0; --i) {
      if (this->container_[i].key_ == key) {
        values.push_back(this->container_[i].value_);
      }
    }
  }
  
  virtual void prepare_threads(const size_t thread_count) final {}

  virtual void register_thread(const size_t thread_id) final {}

  virtual size_t size() const final { return size_; }

protected:
  void base_reorganize() {

    ASSERT(container_ == nullptr && size_ == 0, "invalid container");

    size_t capacity = 0;
    capacity = this->table_ptr_->size();
    
    container_ = new KeyValuePair[capacity];

    DataTableIterator<KeyT, ValueT> iterator(this->table_ptr_);
    while (iterator.has_next()) {
      auto entry = iterator.next();
      container_[size_].key_ = *(entry.key_);
      container_[size_].value_ = entry.offset_;
      ++size_;
    }

    std::sort(container_, container_ + size_, compare_func);

  }

protected:

  KeyValuePair *container_;
  size_t size_;

};
