#pragma once

#include "hash_run.h"
#include <unordered_map>

template<typename KeyT>
class SimpleBtreeRun : public BaseRun<KeyT> {

public:
  SimpleBtreeRun(Storage *storage) : BaseRun<KeyT>(storage) {}

  virtual ~SimpleBtreeRun() {}

  // sort in-memory vector, persist to disk, and clean it up.
  virtual void persist() final {

    if (this->container_.size() == 0) {
      return;
    }

    std::sort(this->container_.begin(), this->container_.end(), BaseRun<KeyT>::compare_func);

  }

  virtual void find(const KeyT key, std::vector<uint64_t> &values) final {

  }

  virtual void print() const {

  }

private:


};
