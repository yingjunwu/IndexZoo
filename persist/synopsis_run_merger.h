#pragma once

#include "base_run_merger.h"

#include "synopsis_run.h"

template<typename KeyT>
class SynopsisRunMerger : public BaseRunMerger<KeyT> {

public:
  SynopsisRunMerger(const std::string &name) : run_(name) {}

  virtual ~SynopsisRunMerger() {}

  void merge(SynopsisRun<KeyT> *lhs_run, SynopsisRun<KeyT> *rhs_run) {

    std::vector<typename BaseRun<KeyT>::KVPair> lhs_container;
    std::vector<typename BaseRun<KeyT>::KVPair> rhs_container;

    lhs_run->cache(lhs_container);
    rhs_run->cache(rhs_container);

    std::vector<typename BaseRun<KeyT>::KVPair> container;
    container.resize(lhs_container.size() + rhs_container.size());

    std::merge(lhs_container.begin(), lhs_container.end(), rhs_container.begin(), rhs_container.end(), container.begin());

    run_.persist(container);
    

  }

  virtual void find(const KeyT key, std::vector<uint64_t> &values) override {
    run_.find(key, values);
  }

  virtual void print() const override {
    run_.print();
  }

private:
  SynopsisRun<KeyT> run_;
};