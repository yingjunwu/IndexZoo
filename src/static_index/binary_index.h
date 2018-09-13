#pragma once

#include <vector>
#include <cmath>

#include "base_static_index.h"

namespace static_index {

template<typename KeyT, typename ValueT>
class BinaryIndex : public BaseStaticIndex<KeyT, ValueT> {

public:
  BinaryIndex(DataTable<KeyT, ValueT> *table_ptr, const size_t num_layers) : BaseStaticIndex<KeyT, ValueT>(table_ptr), num_layers_(num_layers) {}

  virtual ~BinaryIndex() {
    if (num_layers_ != 0) {
      delete[] inner_nodes_;
      inner_nodes_ = nullptr;
    }
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &offsets) final {

    if (this->size_ == 0) {
      return;
    }

    if (key > key_max_ || key < key_min_) {
      return;
    }
    if (key_max_ == key_min_) {
      if (key_max_ == key) {
        for (size_t i = 0; i < this->size_; ++i) {
          offsets.push_back(this->container_[i].offset_);
        }
      }
      return;
    }

    size_t offset_find = this->size_;
    std::pair<int, int> offset_range = find_inner_layers(key);
    if (offset_range.first == offset_range.second) {
      offset_find = offset_range.first;
    } else {
      offset_find = find_internal(key, offset_range.first, offset_range.second);
    }

    if (offset_find == this->size_) {
      // find nothing
      return;
    }

    offsets.push_back(this->container_[offset_find].offset_);

    // move left
    int offset_find_lhs = offset_find - 1;
    while (offset_find_lhs >= 0) {

      if (this->container_[offset_find_lhs].key_ == key) {
        offsets.push_back(this->container_[offset_find_lhs].offset_);
        offset_find_lhs -= 1;
      } else {
        break;
      }
    }
    // move right
    int offset_find_rhs = offset_find + 1;
    while (offset_find_rhs <= this->size_ - 1) {

      if (this->container_[offset_find_rhs].key_ == key) {
        offsets.push_back(this->container_[offset_find_rhs].offset_);
        offset_find_rhs += 1;
      } else {
        break;
      }
    }
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &offsets) final {
    assert(lhs_key < rhs_key);

    if (this->size_ == 0) {
      return;
    }
    if (lhs_key > key_max_ || rhs_key < key_min_) {
      return;
    }

  }

  virtual void reorganize() final {

    this->base_reorganize();

    inner_node_count_ = std::pow(2.0, num_layers_) - 1;

    ASSERT(inner_node_count_ < this->size_, "exceed maximum layers");

    key_min_ = this->container_[0].key_;
    key_max_ = this->container_[this->size_ - 1].key_;
    
    if (num_layers_ != 0) {

      inner_nodes_ = new KeyT[inner_node_count_];
      construct_inner_layers();

    } else {
      inner_nodes_ = nullptr;
    }

  }

  virtual void print() const final {
    if (inner_nodes_ != nullptr) {

      for (size_t i = 0; i < inner_node_count_; ++i) {
        std::cout << inner_nodes_[i] << " ";
      }
      std::cout << std::endl;
    }
  }

private: 

  void construct_inner_layers() {
    ASSERT (num_layers_ != 0, "number of layers cannot be 0");

    size_t begin_offset = 0;
    size_t end_offset = this->size_ - 1;
    size_t mid_offset = (begin_offset + end_offset) / 2;
    
    inner_nodes_[0] = this->container_[mid_offset].key_;
    if (num_layers_ == 1) { return; }

    size_t base_pos = 1;
    size_t next_layer = 1;
    construct_inner_layers_internal(begin_offset, mid_offset - 1, base_pos, 0, next_layer);
    construct_inner_layers_internal(mid_offset + 1, end_offset, base_pos, 1, next_layer);
  }

  void construct_inner_layers_internal(const int begin_offset, const int end_offset, const size_t base_pos, const size_t dst_pos, const size_t curr_layer) {
    if (begin_offset > end_offset) { return; }

    size_t mid_offset = (begin_offset + end_offset) / 2;
  
    ASSERT(base_pos + dst_pos < inner_node_count_, 
      "out of array: " << (base_pos + dst_pos) << " " << inner_node_count_);

    inner_nodes_[base_pos + dst_pos] = this->container_[mid_offset].key_;

    if (num_layers_ == curr_layer + 1) { return; }

    size_t new_base_pos = (base_pos + 1) * 2 - 1;

    construct_inner_layers_internal(begin_offset, mid_offset - 1, new_base_pos, dst_pos * 2, curr_layer + 1);
    construct_inner_layers_internal(mid_offset + 1, end_offset, new_base_pos, dst_pos * 2 + 1, curr_layer + 1);
  }

  // find in leaf nodes, simple binary search
  size_t find_internal(const KeyT &key, const int offset_begin, const int offset_end) {
    if (offset_begin > offset_end) {
      return this->size_;
    }
    int offset_lookup = (offset_begin + offset_end) / 2;
    KeyT key_lookup = this->container_[offset_lookup].key_;
    if (key == key_lookup) {
      return offset_lookup;
    }
    if (key > key_lookup) {
      return find_internal(key, offset_lookup + 1, offset_end);
    } else {
      return find_internal(key, offset_begin, offset_lookup - 1);
    }
  }

  // find in inner nodes
  std::pair<int, int> find_inner_layers(const KeyT &key) {

    if (num_layers_ == 0) { return std::pair<int, int>(0, this->size_); }

    int begin_offset = 0;
    int end_offset = this->size_ - 1;
    int mid_offset = (begin_offset + end_offset) / 2;
    if (key == inner_nodes_[0]) { return std::pair<int, int>(mid_offset, mid_offset); }

    size_t base_pos = 1;
    size_t next_layer = 1;

    if (key < inner_nodes_[0]) {
      return find_inner_layers_internal(key, begin_offset, mid_offset - 1, base_pos, 0, next_layer);
    } else {
      return find_inner_layers_internal(key, mid_offset + 1, end_offset, base_pos, 1, next_layer);
    }

  }

  // helper function for find_inner_layers()
  std::pair<int, int> find_inner_layers_internal(const KeyT &key, const int begin_offset, const int end_offset, const size_t base_pos, const size_t dst_pos, const size_t curr_layer) {

    if (num_layers_ == curr_layer) { return std::pair<int, int>(begin_offset, end_offset); }

    int mid_offset = (begin_offset + end_offset) / 2;
    if (key == inner_nodes_[base_pos + dst_pos]) { return std::pair<int, int>(mid_offset, mid_offset); }

    int new_base_pos = (base_pos + 1) * 2 - 1;

    if (key < inner_nodes_[base_pos + dst_pos]) {
      return find_inner_layers_internal(key, begin_offset, mid_offset - 1, new_base_pos, dst_pos * 2, curr_layer + 1);
    } else {
      return find_inner_layers_internal(key, mid_offset + 1, end_offset, new_base_pos, dst_pos * 2 + 1, curr_layer + 1);
    }

  }

private:

  size_t num_layers_;

  KeyT key_min_;
  KeyT key_max_;
  KeyT *inner_nodes_;
  size_t inner_node_count_;

};

}



