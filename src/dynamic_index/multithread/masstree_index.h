#pragma once

#include <mutex>

#include "masstree/masstree_scan.hh"
#include "masstree/masstree_insert.hh"
#include "masstree/masstree_remove.hh"
#include "masstree/masstree_print.hh"
#include "masstree/timestamp.hh"
#include "masstree/mtcounters.hh"
#include "masstree/circular_int.hh"
#include "masstree/query_masstree.hh"
#include "masstree/kvrow.hh"
#include "masstree/value_bag.hh"
#include "masstree/kvthread.hh"

#include "base_index.h"

extern volatile mrcu_epoch_type active_epoch;

namespace dynamic_index {
namespace multithread {

// thread_local threadinfo *ti_ = NULL;

template<typename KeyT>
class MasstreeIndex : public BaseIndex<KeyT> {
  
  struct masstree_params : public Masstree::nodeparams<> {
    typedef uintptr_t value_type;
    typedef Masstree::value_print<value_type> value_print_type;
    typedef ::threadinfo threadinfo_type;
  };
  typedef Masstree::basic_table<masstree_params> Table;
  typedef lcdf::Str Str;

public:
  MasstreeIndex() {
    main_ti_ = threadinfo::make(threadinfo::TI_MAIN, -1);
    table_.initialize(*main_ti_);
  }

  virtual ~MasstreeIndex() {
  }

  virtual void prepare_threads(const size_t thread_count) final {

  }

  virtual void register_thread(const size_t thread_id) final {

    // std::lock_guard<std::mutex> guard(mutex_);
    // static int idx = 0;
    // if (ti_ == NULL) {
    //   ti_ = threadinfo::make(threadinfo::TI_PROCESS, idx++);
    // }
  }

  virtual void insert(const KeyT &key, const Uint64 &value) final {
    
    // typename Table::cursor_type lp(table_, key->data(), key->len());
    
    // lp.value() = (uintptr_t)key;
    // lp.finish(1, *ti);

  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {

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

private:
  Masstree::basic_table<masstree_params> table_;
  std::mutex mutex_;
  threadinfo *main_ti_;

};

}
}
