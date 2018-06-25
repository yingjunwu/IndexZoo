#pragma once

#include "base_run.h"

template<typename KeyT>
class BaseRunMerger {

public:
  BaseRunMerger() {}

  virtual ~BaseRunMerger() {}

  // virtual void merge(BaseRun *lhs_run, BaseRun *rhs_run) {
  //   assert(is_persisted_ == false);

  //   std::vector<KVPair> lhs_container;
  //   std::vector<KVPair> rhs_container;

  //   lhs_run->cache(lhs_container);
  //   rhs_run->cache(rhs_container);

  //   container_.resize(lhs_container.size() + rhs_container.size());

  //   std::merge(lhs_container.begin(), lhs_container.end(), rhs_container.begin(), rhs_container.end(), container_.begin());

  //   persist();
  // }

};