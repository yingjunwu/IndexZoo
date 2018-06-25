#pragma once

#include "base_run.h"

template<typename KeyT>
class SynopsisRun : public BaseRun<KeyT> {

typedef std::pair<KeyT, KeyT> BoundPair;

public:
  SynopsisRun(const size_t run_id) : BaseRun<KeyT>(run_id) {}

  SynopsisRun(const std::string &run_name) : BaseRun<KeyT>(run_name) {}
  
  virtual ~SynopsisRun() {}

  // sort in-memory vector, persist to disk, and clean it up.
  // NOTE: input container must be sorted beforehand!
  virtual void persist(const std::vector<typename BaseRun<KeyT>::KVPair> &container) override {

    assert(this->is_persisted_ == false);

    KeyT lower_bound, upper_bound;

    uint64_t curr_pos = sizeof(uint64_t);

    bool is_init = true;

    for (auto entry : container) {

      if (curr_pos + sizeof(KeyT) + sizeof(uint64_t) > BLOCK_SIZE) {
        // if exceeds BLOCK_SIZE
        memcpy(this->block_, &curr_pos, sizeof(uint64_t));
        
        uint64_t block_id = this->storage_.write_block(this->block_);
        this->block_ids_.push_back(block_id);
        assert(this->block_bounds_.find(block_id) == this->block_bounds_.end());
        this->block_bounds_[block_id] = BoundPair(lower_bound, upper_bound);

        curr_pos = sizeof(uint64_t);
        is_init = true;
      }

      if (is_init == true) {
        lower_bound = entry.first;
        is_init = false;
      }

      memcpy(this->block_ + curr_pos, &(entry.first), sizeof(KeyT));
      curr_pos += sizeof(KeyT);
      memcpy(this->block_ + curr_pos, &(entry.second), sizeof(uint64_t));
      curr_pos += sizeof(uint64_t);

      upper_bound = entry.first;
    }

    memcpy(this->block_, &curr_pos, sizeof(uint64_t));

    uint64_t block_id = this->storage_.write_block(this->block_);
    this->block_ids_.push_back(block_id);
    assert(this->block_bounds_.find(block_id) == this->block_bounds_.end());
    this->block_bounds_[block_id] = BoundPair(lower_bound, upper_bound);
    
    this->is_persisted_ = true;
  }


  // sort in-memory vector, persist to disk, and clean it up.
  virtual void persist() override {

    if (this->container_.size() == 0) {
      return;
    }

    std::sort(this->container_.begin(), this->container_.end(), BaseRun<KeyT>::compare_func);

    KeyT lower_bound, upper_bound;

    uint64_t curr_pos = sizeof(uint64_t);

    bool is_init = true;

    for (auto entry : this->container_) {

      if (curr_pos + sizeof(KeyT) + sizeof(uint64_t) > BLOCK_SIZE) {
        // if exceeds BLOCK_SIZE
        memcpy(this->block_, &curr_pos, sizeof(uint64_t));
        
        uint64_t block_id = this->storage_.write_block(this->block_);
        this->block_ids_.push_back(block_id);
        assert(this->block_bounds_.find(block_id) == this->block_bounds_.end());
        this->block_bounds_[block_id] = BoundPair(lower_bound, upper_bound);

        curr_pos = sizeof(uint64_t);
        is_init = true;
      }

      if (is_init == true) {
        lower_bound = entry.first;
        is_init = false;
      }

      memcpy(this->block_ + curr_pos, &(entry.first), sizeof(KeyT));
      curr_pos += sizeof(KeyT);
      memcpy(this->block_ + curr_pos, &(entry.second), sizeof(uint64_t));
      curr_pos += sizeof(uint64_t);

      upper_bound = entry.first;
    }

    memcpy(this->block_, &curr_pos, sizeof(uint64_t));

    uint64_t block_id = this->storage_.write_block(this->block_);
    this->block_ids_.push_back(block_id);
    assert(this->block_bounds_.find(block_id) == this->block_bounds_.end());
    this->block_bounds_[block_id] = BoundPair(lower_bound, upper_bound);
    
    this->container_.clear();
    this->is_persisted_ = true;
  }

  virtual void find(const KeyT key, std::vector<uint64_t> &values) override {
    for (auto block_id : this->block_ids_) {

      assert(this->block_bounds_.find(block_id) != this->block_bounds_.end());
      
      BoundPair bounds = this->block_bounds_.at(block_id);
      if (key < bounds.first) {
        return;
      }
      if (key > bounds.second) {
        continue;
      }
      
      // find tuple in block

      this->storage_.read_block(block_id, this->block_);

      // read block
      uint64_t max_pos = 0;
      memcpy(&max_pos, this->block_, sizeof(uint64_t));
      uint64_t curr_pos = sizeof(uint64_t);
      

      while (curr_pos != max_pos) {
        assert(curr_pos < max_pos);

        KeyT load_key;
        uint64_t load_value;

        memcpy(&load_key, this->block_ + curr_pos, sizeof(KeyT));
        curr_pos += sizeof(KeyT);
        memcpy(&load_value, this->block_ + curr_pos, sizeof(uint64_t));
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

  virtual void print() const override {
    std::cout << "=================" << std::endl;
    std::cout << "is persisted: " << (this->is_persisted_ ? "true" : "false") << std::endl;
    std::cout << "number of elements in cache is: " << this->container_.size() << std::endl;

    size_t bound = 5;
    bound = std::min(this->container_.size(), bound);
    for (size_t i = 0; i < bound; ++i) {
      std::cout << this->container_.at(i).first << " " << this->container_.at(i).second << std::endl;
    }

    for (auto entry : this->block_bounds_) {
      std::cout << entry.first << " -> " << "(" << entry.second.first << " " << entry.second.second << ")" << std::endl;
    }

    std::cout << "=================" << std::endl;
  }

private:
  std::unordered_map<uint64_t, BoundPair> block_bounds_;

};




