#pragma once

#include <cassert>
#include <vector>

#include "offset.h"

const uint64_t MaxBlockCapacity = 1000;

class DataBlock {

  public:
    DataBlock(const BlockIDT block_id, const size_t tuple_size, const uint64_t max_block_capacity) : 
      block_id_(block_id),
      tuple_size_(tuple_size), 
      max_rel_offset_(max_block_capacity) {
      
      next_rel_offset_ = 0;

      tuples_ = new char[tuple_size_ * max_rel_offset_];
      memset(tuples_, 0, tuple_size_ * max_rel_offset_);
    }

    ~DataBlock() {
      delete[] tuples_;
      tuples_ = nullptr;
    }

    RelOffsetT get_next_rel_offset() {
      RelOffsetT rel_offset = next_rel_offset_.fetch_add(1);
      if (rel_offset < max_rel_offset_) {
        return rel_offset;
      } else {
        return INVALID_OFFSET;
      }
    }

    char* get_tuple(const RelOffsetT rel_offset) const {
      assert(rel_offset < max_rel_offset_);
      return tuples_ + rel_offset * tuple_size_;
    }

    BlockIDT get_block_id() const {
      return block_id_;
    }

    RelOffsetT get_max_rel_offset() const {
      return max_rel_offset_;
    }

    RelOffsetT getCurrentRelOffset() const {
      return next_rel_offset_.load();
    }

    size_t size() const {
      return next_rel_offset_;
    }

  private:
    DataBlock(const DataBlock &);
    DataBlock& operator=(const DataBlock &);

  private:
    const RelOffsetT max_rel_offset_;
    
    BlockIDT block_id_;

    std::atomic<RelOffsetT> next_rel_offset_;

    size_t tuple_size_;
    char *tuples_;
};

template<typename KeyT, typename ValueT>
class DataTable {
public:
  DataTable(const uint64_t max_block_capacity = MaxBlockCapacity) {

    max_block_capacity_ = max_block_capacity;

    data_blocks_.emplace_back(new DataBlock(0, sizeof(KeyT) + sizeof(ValueT), max_block_capacity_));
    active_data_block_ = data_blocks_.at(0);
  }
  
  ~DataTable() {
    for (auto entry : data_blocks_) {
      delete entry;
      entry = nullptr;
    }
  }

  OffsetT insert_tuple(const KeyT &key, const ValueT &value) {

    while (true) {
      DataBlock* tmp_block = active_data_block_;

      RelOffsetT rel_offset = tmp_block->get_next_rel_offset();

      if (rel_offset != INVALID_OFFSET) {
        
        OffsetT tuple_offset(tmp_block->get_block_id(), rel_offset);

        // copy data.
        char* data = tmp_block->get_tuple(rel_offset);
        memcpy(data, &key, sizeof(key));
        memcpy(data + sizeof(key), &value, sizeof(ValueT));

        if (rel_offset == tmp_block->get_max_rel_offset() - 1) {
          auto new_block = new DataBlock(tmp_block->get_block_id() + 1, sizeof(KeyT) + sizeof(ValueT), max_block_capacity_);
          data_blocks_.emplace_back(new_block);

          COMPILER_MEMORY_FENCE;

          active_data_block_ = new_block;
        }

        return tuple_offset;
      }
    }
  }

  KeyT* get_tuple_key(const OffsetT offset) {

    char *data = data_blocks_.at(offset.block_id())->get_tuple(offset.rel_offset());
    return (KeyT*)(data);
  }

  ValueT* get_tuple_value(const OffsetT offset) {

    char *data = data_blocks_.at(offset.block_id())->get_tuple(offset.rel_offset());
    return (ValueT*)(data + sizeof(KeyT));
  }

  size_t size() const {
    assert(data_blocks_.size() != 0);
    if (data_blocks_.size() == 1) {
      return data_blocks_.at(0)->size();
    } else {
      return (data_blocks_.size() - 1) * max_block_capacity_ + data_blocks_.at(data_blocks_.size() - 1)->size();
    }
  }

  // approximate data table size
  size_t size_approx() const {
    assert(data_blocks_.size() != 0);
    return data_blocks_.size() * max_block_capacity_;
  }

private:
  uint64_t max_block_capacity_;
  std::vector<DataBlock*> data_blocks_;
  DataBlock* active_data_block_;
};
