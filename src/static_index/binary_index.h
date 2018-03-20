#pragma once

namespace static_index {

template<typename KeyT, typename ValueT>
class BinaryIndex : public BaseStaticIndex<KeyT, ValueT> {

public:
  BinaryIndex(DataTable<KeyT, ValueT> *table_ptr) : BaseStaticIndex<KeyT, ValueT>(table_ptr) {

  }

  virtual ~BinaryIndex() {

  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {
    if (key > key_max_ || key < key_min_) {
      return;
    }
    while (true) {

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

  virtual void scan(const KeyT &key, std::vector<Uint64> &values) final {

  }

  virtual void scan_reverse(const KeyT &key, std::vector<Uint64> &values) final {

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