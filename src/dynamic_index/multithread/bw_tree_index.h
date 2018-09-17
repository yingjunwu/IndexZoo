#pragma once

#include "bw_tree/bwtree.h"

#include "base_dynamic_index.h"


namespace dynamic_index {
namespace multithread {

using namespace wangziqi2013::bwtree;

template<typename KeyT, typename ValueT>
class BwTreeIndex : public BaseDynamicIndex<KeyT, ValueT> {

public:
  BwTreeIndex(DataTable<KeyT, ValueT> *table_ptr) : BaseDynamicIndex<KeyT, ValueT>(table_ptr) {
    container_ = new BwTree<KeyT, Uint64>{true};
  }

  virtual ~BwTreeIndex() {
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

  virtual void insert(const KeyT &key, const Uint64 &offset) final {
    container_->Insert(key, offset);
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &offsets) final {
    container_->GetValue(key, offsets);
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &offsets) final {

    if (lhs_key > rhs_key) { return; }

    if (lhs_key == rhs_key) {
      find(lhs_key, offsets);
      return;
    }
    for (auto scan_itr = container_->Begin(lhs_key); (scan_itr.IsEnd() == false) && (container_->KeyCmpLessEqual(scan_itr->first, rhs_key)); scan_itr++) {

      offsets.push_back(scan_itr->second);
    }
  }

  virtual void erase(const KeyT &key) final {
    // container_.erase(key);
  }

  virtual size_t size() const final {
    // return container_.size();
    return 0;
  }

private:
  BwTree<KeyT, Uint64> *container_;
  size_t thread_count_;
};

}
}
