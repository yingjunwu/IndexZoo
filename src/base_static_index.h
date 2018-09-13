#pragma once

#include "base_index.h"

template<typename KeyT, typename ValueT>
class BaseStaticIndex : public BaseIndex<KeyT, ValueT> {

protected:
  struct KeyOffsetPair {
    KeyOffsetPair() : key_(0), offset_(0) {}
    KeyOffsetPair(const KeyT key, const Uint64 offset) : key_(key), offset_(offset) {}

    KeyT key_;
    Uint64 offset_;
  };

  static bool compare_func(KeyOffsetPair &lhs, KeyOffsetPair &rhs) {
    return lhs.key_ < rhs.key_;
  }

public:
  BaseStaticIndex(DataTable<KeyT, ValueT> *table_ptr) : 
    BaseIndex<KeyT, ValueT>(table_ptr), container_(nullptr), size_(0) {}
  
  virtual ~BaseStaticIndex() {
    delete[] container_;
    container_ = nullptr;
  }

  virtual void insert(const KeyT &key, const Uint64 &offset) final {}
  
  virtual void erase(const KeyT &key) final {}

  virtual void scan(const KeyT &key, std::vector<Uint64> &offsets) final {
    for (size_t i = 0; i < this->size_; ++i) {
      if (this->container_[i].key_ == key) {
        offsets.push_back(this->container_[i].offset_);
      }
      if (this->container_[i].key_ > key) {
        return;
      }
    }
  }

  virtual void scan_reverse(const KeyT &key, std::vector<Uint64> &offsets) final {
    for (int i = this->size_ - 1; i >= 0; --i) {
      if (this->container_[i].key_ == key) {
        offsets.push_back(this->container_[i].offset_);
      }
      if (this->container_[i].key_ < key) {
        return;
      }
    }
  }

  virtual void scan_full(std::vector<Uint64> &offsets, const size_t count) final {
    size_t bound = std::min(count, this->size_);
    for (size_t i = 0; i < bound; ++i) {
      offsets.push_back(this->container_[i].offset_);
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
    
    container_ = new KeyOffsetPair[capacity];

    DataTableIterator<KeyT, ValueT> iterator(this->table_ptr_);
    while (iterator.has_next()) {
      auto entry = iterator.next();
      container_[size_].key_ = *(entry.key_);
      container_[size_].offset_ = entry.offset_;
      ++size_;
    }

    std::sort(container_, container_ + size_, compare_func);

  }

protected:

  KeyOffsetPair *container_;
  size_t size_;

};
