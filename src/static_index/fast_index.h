#pragma once

#include <vector>

#include "base_static_index.h"

namespace static_index {

template<typename KeyT, typename ValueT>
class FastIndex : public BaseStaticIndex<KeyT, ValueT> {

  // the FAST index paper assumes SIMD size is 128 bits.
  // It is hard to extend its method to 256/512-bit SIMD
  // as most computers' cacheline size is 64 bytes.
  const size_t SIMD_SIZE = 16; // unit: byte (128 bits)
  const size_t CACHELINE_SIZE = 64; // unit: byte
  const size_t PAGE_SIZE = 4096; // unit: byte (4 KB)

struct BlockLookupInfo {
  size_t current_pos_;
  size_t lhs_offset_;
  size_t rhs_offset_;
};

public:
  FastIndex(DataTable<KeyT, ValueT> *table_ptr, const size_t num_layers): BaseStaticIndex<KeyT, ValueT>(table_ptr), num_layers_(num_layers) {

    ASSERT(sizeof(KeyT) == 4, "only support 4-byte keys");

    // compute size for simd block
    // max simd block key capacity
    simd_key_capacity_ = SIMD_SIZE / sizeof(KeyT);
    // max simd block depth, depth < log2(capacity + 1)
    simd_depth_ = std::log(simd_key_capacity_ + 1) / std::log(2);
    // update simd block key capacity
    simd_key_capacity_ = std::pow(2.0, simd_depth_) - 1;

    // compute size for cacheline block
    // max cacheline block key capacity
    cacheline_key_capacity_ = CACHELINE_SIZE / sizeof(KeyT);
    // max cacheline block depth, depth < log2(capacity + 1)
    cacheline_depth_ = std::log(cacheline_key_capacity_ + 1) / std::log(2);
    // update cacheline block depth and key capacity
    // cacheline_depth % simd_depth must be 0.
    if (cacheline_depth_ % simd_depth_ != 0) {
      cacheline_depth_ = (cacheline_depth_ / simd_depth_) * simd_depth_;
    } 
    cacheline_key_capacity_ = std::pow(2.0, cacheline_depth_) - 1;

    ASSERT(cacheline_key_capacity_ % simd_key_capacity_ == 0, "mismatch: " << cacheline_key_capacity_ << " " << simd_key_capacity_);
    cacheline_capacity_ = cacheline_key_capacity_ / simd_key_capacity_;

    // compute size for page block
    // max page block key capacity
    page_key_capacity_ = PAGE_SIZE / sizeof(KeyT);
    // max page block depth, depth < log2(capacity + 1)
    page_depth_ = std::log(page_key_capacity_ + 1) / std::log(2);
    // update page block depth and key capacity
    // page_depth % cacheline_depth must be 0.
    if (page_depth_ % cacheline_depth_ != 0) {
      page_depth_ = (page_depth_ / cacheline_depth_) * cacheline_depth_;
      page_key_capacity_ = std::pow(2.0, page_depth_) - 1;
    }
    ASSERT(page_key_capacity_ % cacheline_key_capacity_ == 0, "mismatch: " << page_key_capacity_ << " " << cacheline_key_capacity_);
    page_capacity_ = page_key_capacity_ / simd_key_capacity_;

    ASSERT(num_layers_ % cacheline_depth_ == 0, 
      "do not support number of layers = " << num_layers_ << " " << cacheline_depth_);
  }

  virtual ~FastIndex() {
    if (num_layers_ != 0) {
      delete[] inner_nodes_;
      inner_nodes_ = nullptr;
    }
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {

    if (this->size_ == 0) {
      return;
    }

    if (key > key_max_ || key < key_min_) {
      return;
    }
    if (key_max_ == key_min_) {
      if (key_max_ == key) {
        for (size_t i = 0; i < this->size_; ++i) {
          values.push_back(this->container_[i].value_);
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

    values.push_back(this->container_[offset_find].value_);

    // move left
    int offset_find_lhs = offset_find - 1;
    while (offset_find_lhs >= 0) {

      if (this->container_[offset_find_lhs].key_ == key) {
        values.push_back(this->container_[offset_find_lhs].value_);
        offset_find_lhs -= 1;
      } else {
        break;
      }
    }
    // move right
    int offset_find_rhs = offset_find + 1;
    while (offset_find_rhs < this->size_ - 1) {

      if (this->container_[offset_find_rhs].key_ == key) {
        values.push_back(this->container_[offset_find_rhs].value_);
        offset_find_rhs += 1;
      } else {
        break;
      }
    }
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) final {
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

    size_t inner_node_size = std::pow(2.0, num_layers_) - 1;

    ASSERT(inner_node_size < this->size_, "exceed maximum layers");

    key_min_ = this->container_[0].key_;
    key_max_ = this->container_[this->size_ - 1].key_;

    if (num_layers_ != 0) {

      size_t num_cachelines = inner_node_size / cacheline_key_capacity_;
      inner_size_ = num_cachelines * CACHELINE_SIZE / sizeof(KeyT);
      std::cout << "num cachelines = " << num_cachelines << ", inner size = " << inner_size_ << std::endl;
      inner_nodes_ = new KeyT[inner_size_];
      memset(inner_nodes_, 0, sizeof(KeyT) * inner_size_);

      construct_inner_layers();
    } else {

      inner_nodes_ = nullptr;
    }

    print();
  }

  virtual void print() const final {
    if (inner_nodes_ != nullptr) {
      for (size_t i = 0; i < inner_size_; ++i) {
        std::cout << inner_nodes_[i] << " ";
      }
      std::cout << std::endl;
    }
  }

  virtual void print_stats() const final {

  }

private:

  void construct_inner_layers() {
    ASSERT(num_layers_ != 0, "number of layers cannot be 0");

    // level 0
    construct_cacheline_block(0, 0, this->size_ - 1);
    size_t current_pos = 16; // cacheline size

    size_t levels = num_layers_ / cacheline_depth_;

    for (size_t i = 1; i < levels; ++i) {
      // std::cout << "construct levels = " << i << std::endl;
      size_t num_cachelines = std::pow(16, i);
      size_t step = this->size_ / num_cachelines;

      for (size_t j = 0; j < num_cachelines - 1; ++j) {
        construct_cacheline_block(current_pos, step * j, step * (j + 1) - 1);
        current_pos += 16;
      }
      construct_cacheline_block(current_pos, step * (num_cachelines - 1), this->size_ - 1);
      current_pos += 16;
    }
  }
 
  // we only support the case for simd key capacity = 3.
  // in this case, the number of simd blocks in each cacheline block is 5.
  void construct_cacheline_block(const size_t current_pos, const size_t lhs_offset, const size_t rhs_offset) {

    // level 0
    construct_simd_block(current_pos, lhs_offset, rhs_offset);

    size_t step = (rhs_offset - lhs_offset + 1) / 4;

    // level 1
    for (size_t i = 0; i < 3; ++i) {
      construct_simd_block(current_pos + 3 * (i + 1), lhs_offset + step * i, lhs_offset + step * (i + 1) - 1);
    }

    construct_simd_block(current_pos + 3 * 4, lhs_offset + step * 3, rhs_offset);
    
    // CPU fetches data in cacheline size.
  }

  // we only support the case for simd key capacity = 3.
  void construct_simd_block(const size_t current_pos, const size_t lhs_offset, const size_t rhs_offset) {

    ASSERT(simd_key_capacity_ == 3, "SIMD block key capacity not equal to 3: " << simd_key_capacity_);

    size_t step = (rhs_offset - lhs_offset + 1) / 4;

    inner_nodes_[current_pos + 0] = this->container_[lhs_offset + 2 * step].key_;
    inner_nodes_[current_pos + 1] = this->container_[lhs_offset + 1 * step].key_;
    inner_nodes_[current_pos + 2] = this->container_[lhs_offset + 3 * step].key_;

  }

  // find in inner nodes
  std::pair<int, int> find_inner_layers(const KeyT &key) {

    if (num_layers_ == 0) { return std::pair<int, int>(0, this->size_ - 1); }

    BlockLookupInfo info;
    // cacheline level 0
    size_t branch_id = lookup_cacheline_block(key, 0, 0, this->size_ - 1, info);
    // return std::pair<int, int>(info.lhs_offset_, info.rhs_offset_);

    size_t levels = num_layers_ / cacheline_depth_;

    if (levels == 2) {

      size_t step = this->size_ / 16;

      size_t new_pos = CACHELINE_SIZE / sizeof(KeyT) * (1 + branch_id);

      // cacheline level 1
      size_t new_branch_id;
      if (branch_id < 15) {

        new_branch_id = lookup_cacheline_block(key, new_pos, branch_id * step, (branch_id + 1) * step - 1, info);
        // std::cout << "offset: " << branch_id * step << " " << (branch_id + 1) * step - 1 << std::endl;
      } else {
        new_branch_id = lookup_cacheline_block(key, new_pos, branch_id * step, this->size_ - 1, info);
        // std::cout << "offset: " << branch_id * step << " " << this->size_ - 1 << std::endl;
      }
      // std::cout << "output: " << key << " " << step << " " << new_pos << " " << branch_id << " " << new_branch_id << " " << info.lhs_offset_ << " " << info.rhs_offset_ << std::endl;
      
    }
    return std::pair<int, int>(info.lhs_offset_, info.rhs_offset_);
  }

  // search in cacheline block
  size_t lookup_cacheline_block(const KeyT &key, const size_t current_pos, const size_t lhs_offset, const size_t rhs_offset, BlockLookupInfo &info) {

    size_t branch_id = lookup_simd_block(key, current_pos, lhs_offset, rhs_offset, info);
    
    size_t new_pos = current_pos + 3 * (branch_id + 1);

    size_t step = (rhs_offset - lhs_offset + 1) / 4;

    size_t new_branch_id;
    if (branch_id < 3) {

      new_branch_id = lookup_simd_block(key, new_pos, lhs_offset + step * branch_id, lhs_offset + step * (branch_id + 1) - 1, info); 
    } else {
      new_branch_id = lookup_simd_block(key, new_pos, lhs_offset + step * branch_id, rhs_offset, info);       
    }

    return branch_id * 4 + new_branch_id;
  }

  // search in simd block
  size_t lookup_simd_block(const KeyT &key, const size_t current_pos, const size_t lhs_offset, const size_t rhs_offset, BlockLookupInfo &info) {
    // std::cout << "key = " << key << ", lookup simd: " << lhs_offset << " " << rhs_offset << std::endl;

    size_t step = (rhs_offset - lhs_offset + 1) / 4;

    if (key >= inner_nodes_[current_pos + 2]) {
      info.current_pos_ = current_pos + 3 * 4;
      info.lhs_offset_ = lhs_offset + 3 * step;
      info.rhs_offset_ = rhs_offset;
      return 3;
    }
    else if (key >= inner_nodes_[current_pos + 0]) {
      info.current_pos_ = current_pos + 3 * 3;
      info.lhs_offset_ = lhs_offset + 2 * step;
      info.rhs_offset_ = lhs_offset + 3 * step - 1;
      return 2;
    } 
    else if (key >= inner_nodes_[current_pos + 1]) { 
      info.current_pos_ = current_pos + 3 * 2;
      info.lhs_offset_ = lhs_offset + 1 * step;
      info.rhs_offset_ = lhs_offset + 2 * step - 1;
      return 1;
    } 
    else {
      info.current_pos_ = current_pos + 3 * 1;
      info.lhs_offset_ = lhs_offset;
      info.rhs_offset_ = lhs_offset + 1 * step - 1;
      return 0;
    }
  }


  // last step
  // find in leaf nodes, simple binary search [incl., incl.]
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

private:
  
  size_t num_layers_;

  KeyT key_min_;
  KeyT key_max_;
  KeyT *inner_nodes_;
  size_t inner_size_;

  size_t simd_key_capacity_;
  size_t simd_depth_;

  size_t cacheline_key_capacity_;
  size_t cacheline_depth_;
  size_t cacheline_capacity_;

  size_t page_key_capacity_;
  size_t page_depth_;
  size_t page_capacity_;

};

}