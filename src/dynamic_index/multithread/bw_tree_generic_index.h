#pragma once

#include "bw_tree/bwtree.h"

#include "base_dynamic_generic_index.h"


namespace dynamic_index {
namespace multithread {

using namespace wangziqi2013::bwtree;

template<typename ValueT>
class BwTreeGenericIndex : public BaseDynamicGenericIndex<ValueT> {

public:
  BwTreeGenericIndex(GenericDataTable<ValueT> *table_ptr) : BaseDynamicGenericIndex<ValueT>(table_ptr) {
    container_ = new BwTree<GenericKey, Uint64, GenericKeyComparator, GenericKeyEqualityChecker, GenericKeyHasher>{true};
  }

  virtual ~BwTreeGenericIndex() {
    delete container_;
    container_ = nullptr;
  }

  virtual void prepare_threads(const size_t thread_count) final {
    thread_count_ = thread_count;
    container_->UpdateThreadLocal(thread_count_);
  }

  virtual void register_thread(const size_t thread_id) final {
    assert(thread_id < thread_count_);
    container_->AssignGCID(thread_id);
  }

  virtual void insert(const GenericKey &key, const Uint64 &value) final {
    container_->Insert(key, value);
  }

  virtual void find(const GenericKey &key, std::vector<Uint64> &values) final {
    container_->GetValue(key, values);
  }

  virtual void find_range(const GenericKey &lhs_key, const GenericKey &rhs_key, std::vector<Uint64> &values) final {

    if (lhs_key > rhs_key) { return; }

    if (lhs_key == rhs_key) {
      find(lhs_key, values);
      return;
    }
    for (auto scan_itr = container_->Begin(lhs_key); (scan_itr.IsEnd() == false) && (container_->KeyCmpLessEqual(scan_itr->first, rhs_key)); scan_itr++) {

      values.push_back(scan_itr->second);
    }
  }

  virtual void erase(const GenericKey &key) final {
    // container_.erase(key);
  }

  virtual size_t size() const final {
    // return container_.size();
    return 0;
  }

private:
  BwTree<GenericKey, Uint64, GenericKeyComparator, GenericKeyEqualityChecker, GenericKeyHasher> *container_;
  size_t thread_count_;
};

}
}
