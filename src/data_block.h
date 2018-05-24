#pragma once

#include <atomic>
#include <cstring>

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
