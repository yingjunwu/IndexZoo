#pragma once

#include "base_run_merger.h"

#include "hash_run.h"

template<typename KeyT>
class LogicalHashRunMerger : public BaseRunMerger<KeyT> {

typedef std::pair<HashRun<KeyT>*, OffsetT> PosPair;

public:
  LogicalHashRunMerger(const std::string &name) {}

  virtual ~LogicalHashRunMerger() {}

  void merge(HashRun<KeyT> *lhs_run, HashRun<KeyT> *rhs_run) {

    for (auto entry : lhs_run->hash_table_) {
      hash_table_.insert(std::pair<KeyT, PosPair>(entry.first, PosPair(lhs_run, entry.second)));
    }

    for (auto entry : rhs_run->hash_table_) {
      hash_table_.insert(std::pair<KeyT, PosPair>(entry.first, PosPair(rhs_run, entry.second)));
    }
  }

  virtual void find(const KeyT key, std::vector<uint64_t> &values) override {

    auto iter_range = hash_table_.equal_range(key);
    
    for (auto iter = iter_range.first; iter != iter_range.second; ++iter) {
      iter->second.first->find(key, values);
    }
  }

  virtual void print() const override {
    std::cout << "=================" << std::endl;

    size_t bound = 5;
    bound = std::min(hash_table_.size(), bound);

    size_t curr = 0;
    for (auto entry : hash_table_) {
      if (curr >= bound) {
        break;
      }
      std::cout << entry.first << " -> " << "(" << entry.second.first << " " << entry.second.second << ")" << std::endl;

      ++curr;
    }

    std::cout << "=================" << std::endl;
  }

private:

  std::unordered_multimap<KeyT, PosPair> hash_table_;

};
