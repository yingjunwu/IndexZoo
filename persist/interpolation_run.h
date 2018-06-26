#pragma once

#include "base_run.h"
#include <unordered_map>

template<typename KeyT>
class InterpolationRun : public BaseRun<KeyT> {

public:
  InterpolationRun(const size_t run_id) : BaseRun<KeyT>(run_id) {}

  InterpolationRun(const std::string &run_name) : BaseRun<KeyT>(run_name) {}

  virtual ~InterpolationRun() {}

    // sort in-memory vector, persist to disk, and clean it up.
  virtual void persist() override {

    if (this->container_.size() == 0) {
      return;
    }

    std::sort(this->container_.begin(), this->container_.end(), BaseRun<KeyT>::compare_func);

    KeyT lower_bound = this->container_.at(0);

    KeyT upper_bound = this->container_.at(this->container_.size() - 1);

    // reset curr_pos
    uint64_t curr_pos = sizeof(uint64_t);

    for (auto entry : this->container_) {

      if (curr_pos + sizeof(KeyT) + sizeof(uint64_t) > BLOCK_SIZE) {
        // if exceeds BLOCK_SIZE
        // write block size to block header
        memcpy(this->block_, &curr_pos, sizeof(uint64_t));

        // record block id
        uint64_t block_id = this->storage_.write_block(this->block_);
        this->block_ids_.push_back(block_id);

        // reset curr_pos
        curr_pos = sizeof(uint64_t);
      }

      // only index the first value mapped by a key
      if (hash_table_.find(entry.first) == hash_table_.end()) {
        hash_table_[entry.first] = OffsetT(this->storage_.next_block_id(), curr_pos);
      }

      memcpy(this->block_ + curr_pos, &(entry.first), sizeof(KeyT));
      curr_pos += sizeof(KeyT);
      memcpy(this->block_ + curr_pos, &(entry.second), sizeof(uint64_t));
      curr_pos += sizeof(uint64_t);

    }

    // write block size to block header
    memcpy(this->block_, &curr_pos, sizeof(uint64_t));

    // record block id
    uint64_t block_id = this->storage_.write_block(this->block_);
    this->block_ids_.push_back(block_id);

    this->container_.clear();
    this->is_persisted_ = true;

  }
  std::unordered_map<KeyT, OffsetT> hash_table_;
};


