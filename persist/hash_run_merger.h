#pragma once

#include "base_run_merger.h"

#include "base_run.h"
#include "hash_run.h"

template<typename KeyT>
class HashRunMerger : public BaseRunMerger<KeyT> {

public:
  HashRunMerger(const std::string name) : run_(name) {}

  virtual ~HashRunMerger() {}

  void merge(HashRun<KeyT> *lhs_run, HashRun<KeyT> *rhs_run) {

    std::vector<typename BaseRun<KeyT>::KVPair> lhs_container;
    std::vector<typename BaseRun<KeyT>::KVPair> rhs_container;

    lhs_run->cache(lhs_container);
    rhs_run->cache(rhs_container);

    std::vector<typename BaseRun<KeyT>::KVPair> container;
    container.resize(lhs_container.size() + rhs_container.size());

    std::merge(lhs_container.begin(), lhs_container.end(), rhs_container.begin(), rhs_container.end(), container.begin());

    run_.persist(container);
    

  }

  virtual void find(const KeyT key, std::vector<uint64_t> &values) {
    run_.find(key, values);
  }

  virtual void print() const {
    run_.print();
  }

private:
  HashRun<KeyT> run_;
};