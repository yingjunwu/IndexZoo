#pragma once

#include "bw_tree/bwtree.h"

#include "base_index.h"

using namespace wangziqi2013::bwtree;

template<typename KeyT>
class BwTreeIndex : public BaseIndex<KeyT> {

public:
  BwTreeIndex(const size_t size_hint) {
    container_ = new BwTree<KeyT, Uint64>{true};
  }

  BwTreeIndex() {
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

  virtual void insert(const KeyT &key, const Uint64 &value) final {
    container_->Insert(key, value);
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {
    container_->GetValue(key, values);
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) final {
    // assert(false);
  }

  virtual void erase(const KeyT &key) final {
    // container_.erase(key);
  }

  virtual size_t size() const final {
    // return container_.size();
    return 0;
  }

public:
  BwTree<KeyT, Uint64> *container_;
  size_t thread_count_;
};
