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
  BaseRun(const size_t run_id) : 
    storage_("run" + std::to_string(run_id) + ".dat"), 
    block_(new char[BLOCK_SIZE]),
    is_persisted_(false) {}

  virtual ~BaseRun() {
    delete[] block_;
    block_ = nullptr;
  }

  void insert(const KeyT key, const uint64_t value) {
    container_.push_back(KVPair(key, value));
  }

  virtual void merge(BaseRun *lhs_run, BaseRun *rhs_run) {
    assert(is_persisted_ == false);

    std::vector<KVPair> lhs_container;
    std::vector<KVPair> rhs_container;

    lhs_run->cache(lhs_container);
    rhs_run->cache(rhs_container);

    container_.resize(lhs_container.size() + rhs_container.size());

    std::merge(lhs_container.begin(), lhs_container.end(), rhs_container.begin(), rhs_container.end(), container_.begin());

    persist();
  }

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

  // sort in-memory vector, persist to disk, and clean it up.
  virtual void persist() {

    if (container_.size() == 0) {
      return;
    }

    std::sort(container_.begin(), container_.end(), compare_func);

    uint64_t curr_pos = sizeof(uint64_t);

    for (auto entry : container_) {

      if (curr_pos + sizeof(KeyT) + sizeof(uint64_t) > BLOCK_SIZE) {
        // if exceeds BLOCK_SIZE
        memcpy(block_, &curr_pos, sizeof(uint64_t));
        
        uint64_t block_id = storage_.write_block(block_);
        block_ids_.push_back(block_id);

        curr_pos = sizeof(uint64_t);
      }

      memcpy(block_ + curr_pos, &(entry.first), sizeof(KeyT));
      curr_pos += sizeof(KeyT);
      memcpy(block_ + curr_pos, &(entry.second), sizeof(uint64_t));
      curr_pos += sizeof(uint64_t);
    }

    memcpy(block_, &curr_pos, sizeof(uint64_t));

    uint64_t block_id = storage_.write_block(block_);
    block_ids_.push_back(block_id);
    
    container_.clear();
    is_persisted_ = true;
  }

  virtual void find(const KeyT key, std::vector<uint64_t> &values) {
    for (auto block_id : block_ids_) {
      
      storage_.read_block(block_id, block_);

      // read block
      uint64_t max_pos = 0;
      memcpy(&max_pos, block_, sizeof(uint64_t));
      uint64_t curr_pos = sizeof(uint64_t);
      

      while (curr_pos != max_pos) {
        assert(curr_pos < max_pos);

        KeyT load_key;
        uint64_t load_value;

        memcpy(&load_key, block_ + curr_pos, sizeof(KeyT));
        curr_pos += sizeof(KeyT);
        memcpy(&load_value, block_ + curr_pos, sizeof(uint64_t));
        curr_pos += sizeof(uint64_t);

        if (load_key > key) {
          return;
        }

        if (load_key == key) {
          values.push_back(load_value);
        }
      }
    }
  }


  virtual void print() const {
    std::cout << "=================" << std::endl;
    std::cout << "is persisted: " << (this->is_persisted_ ? "true" : "false") << std::endl;
    std::cout << "number of elements in cache is: " << this->container_.size() << std::endl;

    size_t bound = 5;
    bound = std::min(this->container_.size(), bound);
    for (size_t i = 0; i < bound; ++i) {
      std::cout << this->container_.at(i).first << " " << this->container_.at(i).second << std::endl;
    }

    std::cout << "=================" << std::endl;
  }
  
  
protected:
  std::vector<KVPair> container_;
  Storage storage_;
  char *block_; // buffer for holding to-be-persisted data
  std::vector<uint64_t> block_ids_;

  bool is_persisted_;
};








