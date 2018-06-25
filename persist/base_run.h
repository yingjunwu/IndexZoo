#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <typeinfo>

#include "storage.h"

template<typename KeyT>
class BaseRun {

protected:
typedef std::pair<KeyT, uint64_t> KVPair;

static bool compare_func(KVPair &lhs, KVPair &rhs) {
  return lhs.first < rhs.first;
}

public:
  BaseRun(const size_t run_id) : BaseRun("run" + std::to_string(run_id) + ".dat") {} 

  BaseRun(const std::string &run_name) :
    storage_(run_name),
    block_(new char[BLOCK_SIZE]),
    is_persisted_(false) {}

  virtual ~BaseRun() {
    delete[] block_;
    block_ = nullptr;
  }

  void insert(const KeyT key, const uint64_t value) {
    container_.push_back(KVPair(key, value));
  }

  // fetch data from disk and store it into the input vector.
  void cache(std::vector<KVPair> &container) {
    for (auto block_id : block_ids_) {

      storage_.read_block(block_id, block_);

      // read block
      uint64_t max_pos = 0;
      memcpy(&max_pos, block_, sizeof(uint64_t));
      uint64_t curr_pos = sizeof(uint64_t);
      
      while (curr_pos != max_pos) {
        assert(curr_pos < max_pos);

        KeyT key;
        uint64_t value;

        memcpy(&key, block_ + curr_pos, sizeof(KeyT));
        curr_pos += sizeof(KeyT);
        memcpy(&value, block_ + curr_pos, sizeof(uint64_t));
        curr_pos += sizeof(uint64_t);

        container.push_back(KVPair(key, value));
      }

    }
  }

  // fetch data from disk and store it in in-memory vector.
  void cache() {
    for (auto block_id : block_ids_) {

      storage_.read_block(block_id, block_);

      // read block
      uint64_t max_pos = 0;
      memcpy(&max_pos, block_, sizeof(uint64_t));
      uint64_t curr_pos = sizeof(uint64_t);
      
      while (curr_pos != max_pos) {
        assert(curr_pos < max_pos);

        KeyT key;
        uint64_t value;

        memcpy(&key, block_ + curr_pos, sizeof(KeyT));
        curr_pos += sizeof(KeyT);
        memcpy(&value, block_ + curr_pos, sizeof(uint64_t));
        curr_pos += sizeof(uint64_t);

        container_.push_back(KVPair(key, value));
      }

    }
    is_persisted_ = false;
  }

  virtual void persist(const std::vector<KVPair> &container) = 0;

  virtual void persist() = 0;

  virtual void find(const KeyT key, std::vector<uint64_t> &values) = 0;
  
  virtual void print() const = 0;

protected:
  std::vector<KVPair> container_;
  Storage storage_;
  char *block_; // buffer for holding to-be-persisted data
  std::vector<uint64_t> block_ids_;

  bool is_persisted_;
};








