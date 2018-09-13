#pragma once

#include <cassert>
#include <vector>

#include "data_block.h"

template<typename KeyT, typename ValueT>
class DataTableIterator;

template<typename KeyT, typename ValueT>
class DataTable {

  friend DataTableIterator<KeyT, ValueT>;

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

  KeyT* get_tuple_key(const BlockIDT block_id, const RelOffsetT rel_offset) const {

    char *data = data_blocks_.at(block_id)->get_tuple(rel_offset);
    return (KeyT*)(data);
  }

  ValueT* get_tuple_value(const BlockIDT block_id, const RelOffsetT rel_offset) const {

    char *data = data_blocks_.at(block_id)->get_tuple(rel_offset);
    return (ValueT*)(data + sizeof(KeyT));
  }

  KeyT* get_tuple_key(const OffsetT offset) const {

    char *data = data_blocks_.at(offset.block_id())->get_tuple(offset.rel_offset());
    return (KeyT*)(data);
  }

  ValueT* get_tuple_value(const OffsetT offset) const {

    char *data = data_blocks_.at(offset.block_id())->get_tuple(offset.rel_offset());
    return (ValueT*)(data + sizeof(KeyT));
  }

  size_t size() const {
    ASSERT(data_blocks_.size() != 0, "must have at least one data block");
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

template<typename KeyT, typename ValueT>
class DataTableIterator {

public:
  struct IteratorEntry {
    IteratorEntry(const BlockIDT block_id, const RelOffsetT rel_offset, KeyT *key, ValueT *value) : 
      offset_(OffsetT::construct_raw_data(block_id, rel_offset)), key_(key), value_(value) {}

    Uint64 offset_;
    KeyT* key_;
    ValueT* value_;
  };

public:
  DataTableIterator(DataTable<KeyT, ValueT> *table_ptr) : 
    table_ptr_(table_ptr), curr_block_id_(0), curr_rel_offset_(0) {
    
    ASSERT(table_ptr_->data_blocks_.size() != 0, "table must contain at least one data block!");
    ASSERT(!(table_ptr_->data_blocks_.size() == 1 && table_ptr_->data_blocks_.at(0)->size() == 0), "table must contain at least one tuple!");

    max_rel_offset_ = table_ptr_->max_block_capacity_ - 1; 

    last_block_id_ = table_ptr_->data_blocks_.size() - 1;

    size_t last_block_size = table_ptr_->data_blocks_.at(last_block_id_)->size();
    if (last_block_size == 0) {
      last_rel_offset_ = max_rel_offset_;
      last_block_id_ = last_block_id_ - 1;
    } else {
      last_rel_offset_ = last_block_size - 1;
    }
  }

  bool has_next() const {
    if (curr_block_id_ > last_block_id_ || (curr_block_id_ == last_block_id_ && curr_rel_offset_ > last_rel_offset_)) {
      return false;
    } else {
      return true;
    }
  }

  IteratorEntry next() {
    BlockIDT ret_block_id = curr_block_id_;
    RelOffsetT ret_rel_offset = curr_rel_offset_;

    if (curr_rel_offset_ != max_rel_offset_) {
      curr_rel_offset_++;
    } else {
      curr_block_id_++;
      curr_rel_offset_ = 0;
    }
    return IteratorEntry(ret_block_id, ret_rel_offset, table_ptr_->get_tuple_key(ret_block_id, ret_rel_offset), table_ptr_->get_tuple_value(ret_block_id, ret_rel_offset));
  }


private:
  DataTable<KeyT, ValueT> *table_ptr_;

  BlockIDT curr_block_id_;
  RelOffsetT curr_rel_offset_;

  BlockIDT last_block_id_;
  RelOffsetT last_rel_offset_;
  RelOffsetT max_rel_offset_;
};
