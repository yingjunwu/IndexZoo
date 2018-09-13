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

#include "base_dynamic_generic_index.h"

extern volatile uint64_t globalepoch;
extern volatile bool recovering;
extern thread_local threadinfo *ti_;

namespace dynamic_index {
namespace multithread {


class MasstreeGenericIndex : public BaseDynamicGenericIndex {

public:
  MasstreeGenericIndex(GenericDataTable *table_ptr) : BaseDynamicGenericIndex(table_ptr) {
    container_ = new Masstree::default_table();

    threadinfo *main_ti = threadinfo::make(threadinfo::TI_MAIN, -1);
    container_->initialize(*main_ti);
  }

  virtual ~MasstreeGenericIndex() {
    delete container_;
    container_ = nullptr;
  }

  virtual void prepare_threads(const size_t thread_count) final {}

  virtual void register_thread(const size_t thread_id) final {

    std::lock_guard<std::mutex> guard(mutex_);
    static int idx = 0;
    if (ti_ == nullptr) {
      ti_ = threadinfo::make(threadinfo::TI_PROCESS, idx++);
    }
  }

  virtual void insert(const GenericKey &key, const Uint64 &offset) final {
    
    typename Masstree::default_table::cursor_type lp(container_->table(), key.raw(), key.size());
    bool found = lp.find_insert(*ti_);
    if (!found) {
      ti_->advance_timestamp(lp.node_timestamp());
      qtimes_.ts = ti_->update_timestamp();
      qtimes_.prev_ts = 0;
    }
    else {
      qtimes_.ts = ti_->update_timestamp(lp.value()->timestamp());
      qtimes_.prev_ts = lp.value()->timestamp();
      lp.value()->deallocate_rcu(*ti_);
    }
    
    lp.value() = row_type::create1(Str((char*)(&offset), sizeof(offset)), qtimes_.ts, *ti_);
    lp.finish(1, *ti_);

  }

  virtual void find(const GenericKey &key, std::vector<Uint64> &offsets) final {

    Str value;
    typename Masstree::default_table::unlocked_cursor_type lp(container_->table(), key.raw(), key.size());
    bool found = lp.find_unlocked(*ti_);
    if (found) {
      value = lp.value()->col(0);
      offsets.push_back(*(Uint64*)(value.s));
    }
  }

  virtual void find_range(const GenericKey &lhs_key, const GenericKey &rhs_key, std::vector<Uint64> &offsets) final {
    // assert(false);
  }

  virtual void erase(const GenericKey &key) final {
    // container_.erase(key);
  }

  virtual size_t size() const final {
    // return container_.size();
    return 0;
  }

private:
    Masstree::default_table *container_;
    std::mutex mutex_;
    loginfo::query_times qtimes_;
};

}
}
