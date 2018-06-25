#pragma once

#include "base_run.h"
#include <unordered_map>

template<typename KeyT>
class LogicalHashRun : public BaseRun<KeyT> {

public:
  LogicalHashRun(const size_t run_id) : BaseRun<KeyT>(run_id) {}

  virtual ~LogicalHashRun() {}

  // virtual void merge(BaseRun<KeyT> *lhs_run, BaseRun<KeyT> *rhs_run) override {
  //   assert(this->is_persisted_ == false);

  //   if (typeid(*lhs_run) == typeid(LogicalHashRun) && typeid(*rhs_run) == typeid(LogicalHashRun)) {



  //   } else {

  //     std::vector<typename BaseRun<KeyT>::KVPair> lhs_container;
  //     std::vector<typename BaseRun<KeyT>::KVPair> rhs_container;

  //     lhs_run->cache(lhs_container);
  //     rhs_run->cache(rhs_container);

  //     this->container_.resize(lhs_container.size() + rhs_container.size());

  //     std::merge(lhs_container.begin(), lhs_container.end(), rhs_container.begin(), rhs_container.end(), this->container_.begin());

  //     // persist();
  //   }
  // }

  // // sort in-memory vector, persist to disk, and clean it up.
  // virtual void persist() {

  //   if (container_.size() == 0) {
  //     return;
  //   }

  //   std::sort(container_.begin(), container_.end(), compare_func);

  //   uint64_t curr_pos = sizeof(uint64_t);

  //   for (auto entry : container_) {

  //     if (curr_pos + sizeof(KeyT) + sizeof(uint64_t) > BLOCK_SIZE) {
  //       // if exceeds BLOCK_SIZE
  //       memcpy(block_, &curr_pos, sizeof(uint64_t));
        
  //       uint64_t block_id = storage_.write_block(block_);
  //       block_ids_.push_back(block_id);

  //       curr_pos = sizeof(uint64_t);
  //     }

  //     memcpy(block_ + curr_pos, &(entry.first), sizeof(KeyT));
  //     curr_pos += sizeof(KeyT);
  //     memcpy(block_ + curr_pos, &(entry.second), sizeof(uint64_t));
  //     curr_pos += sizeof(uint64_t);
  //   }

  //   memcpy(block_, &curr_pos, sizeof(uint64_t));

  //   uint64_t block_id = storage_.write_block(block_);
  //   block_ids_.push_back(block_id);
    
  //   container_.clear();
  //   is_persisted_ = true;
  // }

  virtual void find(const KeyT key, std::vector<uint64_t> &values) override {
    OffsetT pos = hash_table_.find(key)->second;
    uint64_t curr_pos = pos.rel_offset();

    this->storage_.read_block(pos.block_id(), this->block_);
    KeyT load_key;
    uint64_t load_value;

    memcpy(&load_key, this->block_ + curr_pos, sizeof(KeyT));
    memcpy(&load_value, this->block_ + curr_pos + sizeof(KeyT), sizeof(uint64_t));

    values.push_back(load_value);

  }

private:
  std::unordered_multimap<KeyT, OffsetT> hash_table_;

};
