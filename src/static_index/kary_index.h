#pragma once

namespace static_index {

template<typename KeyT, typename ValueT>
class KAryIndex : public BaseStaticIndex<KeyT, ValueT> {

public:
  KAryIndex(DataTable<KeyT, ValueT> *table_ptr) : BaseStaticIndex<KeyT, ValueT>(table_ptr) {

  }

  virtual ~KAryIndex() {

  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {

  }

  virtual void scan(const KeyT &key, std::vector<Uint64> &values) final {

  }

  virtual void scan_reverse(const KeyT &key, std::vector<Uint64> &values) final {

  }

  virtual void reorganize() final {

  }

  virtual void print() const final {

  }

  virtual void print_stats() const final {

  }

private:
  

};

}