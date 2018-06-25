#pragma once

#include "base_run.h"
#include <unordered_map>

template<typename KeyT>
class LogicalHashRunMerger;

template<typename KeyT>
class HashRun : public BaseRun<KeyT> {

friend class LogicalHashRunMerger<KeyT>;

public:
  HashRun(const size_t run_id) : BaseRun<KeyT>(run_id) {}

  HashRun(const std::string &run_name) : BaseRun<KeyT>(run_name) {}

  virtual ~HashRun() {}

  // sort in-memory vector, persist to disk, and clean it up.
  // NOTE: input container must be sorted beforehand!
  virtual void persist(const std::vector<typename BaseRun<KeyT>::KVPair> &container) override {

    assert(this->is_persisted_ == false);

    // reset curr_pos
    uint64_t curr_pos = sizeof(uint64_t);

    for (auto entry : container) {

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

    this->is_persisted_ = true;
  }

  // sort in-memory vector, persist to disk, and clean it up.
  virtual void persist() override {

    if (this->container_.size() == 0) {
      return;
    }

    std::sort(this->container_.begin(), this->container_.end(), BaseRun<KeyT>::compare_func);

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

  virtual void find(const KeyT key, std::vector<uint64_t> &values) override {
    if (hash_table_.find(key) == hash_table_.end()) {
      return;
    }
    OffsetT pos = hash_table_.at(key);
    uint64_t curr_pos = pos.rel_offset();

    this->storage_.read_block(pos.block_id(), this->block_);
    KeyT load_key;
    uint64_t load_value;


    memcpy(&load_key, this->block_ + curr_pos, sizeof(KeyT));
    memcpy(&load_value, this->block_ + curr_pos + sizeof(KeyT), sizeof(uint64_t));

    values.push_back(load_value);

  }

  virtual void print() const override {
    std::cout << "=================" << std::endl;
    std::cout << "is persisted: " << (this->is_persisted_ ? "true" : "false") << std::endl;
    std::cout << "number of elements in cache is: " << this->container_.size() << std::endl;

    size_t bound = 5;
    bound = std::min(this->container_.size(), bound);
    for (size_t i = 0; i < bound; ++i) {
      std::cout << this->container_.at(i).first << " " << this->container_.at(i).second << std::endl;
    }

    size_t curr = 0;
    for (auto entry : this->hash_table_) {
      if (curr >= bound) {
        break;
      }
      std::cout << entry.first << " -> " << entry.second << std::endl;

      ++curr;
    }

    std::cout << "=================" << std::endl;


  }

private:
  std::unordered_map<KeyT, OffsetT> hash_table_;

};
