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


public:
  FastIndex(DataTable<KeyT, ValueT> *table_ptr, const size_t num_layers): BaseStaticIndex<KeyT, ValueT>(table_ptr), num_layers_(num_layers) {

    ASSERT(sizeof(KeyT) == 4, "only support 4-byte keys");

    // compute size for simd block
    simd_key_capacity_ = SIMD_SIZE / sizeof(KeyT) - 1;
    simd_depth_ = std::log(simd_key_capacity_ + 1) / std::log(2);
    // currently only support simd_key_capacity = 3.
    // ASSERT(simd_key_capacity_ == 3, 
    //   "do not support key size = " << sizeof(KeyT) << " bytes");

    // compute size for cacheline block
    cacheline_key_capacity_ = CACHELINE_SIZE / sizeof(KeyT) - 1;
    cacheline_depth_ = std::log(cacheline_key_capacity_ + 1) / std::log(2);
    if (cacheline_depth_ % simd_depth_ != 0) {
      cacheline_depth_ = (cacheline_depth_ / simd_depth_) * simd_depth_;
      cacheline_key_capacity_ = std::pow(2.0, cacheline_depth_) - 1;
    }

    // compute size for page block
    page_key_capacity_ = PAGE_SIZE / sizeof(KeyT) - 1;
    page_depth_ = std::log(page_key_capacity_ + 1) / std::log(2);
    if (page_depth_ % cacheline_depth_ != 0) {
      page_depth_ = (page_depth_ / cacheline_depth_) * cacheline_depth_;
      page_key_capacity_ = std::pow(2.0, page_depth_) - 1;
    }

    std::cout << "simd: " << simd_key_capacity_ << " " << simd_depth_ << std::endl;
    std::cout << "cacheline: " << cacheline_key_capacity_ << " " << cacheline_depth_ << std::endl;
    std::cout << "page: " << page_key_capacity_ << " " << page_depth_ << std::endl;

    ASSERT(num_layers_ % page_depth_ == 0, 
      "do not support number of layers = " << num_layers_ << " " << page_depth_);

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
      // size_t inner_size = std::pow(2, num_layers_) - 1;
      inner_size_ = 272;
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
      // size_t inner_size = std::pow(2, num_layers_) - 1;
      for (size_t i = 0; i < inner_size_; ++i) {
        std::cout << inner_nodes_[i] << " ";
      }
      std::cout << std::endl;
    }
  }

  virtual void print_stats() const final {

  }

private:

  size_t median(const size_t lhs, const size_t rhs) { return (lhs + rhs) / 2; }

  void construct_inner_layers() {
    ASSERT(num_layers_ != 0, "number of layers cannot be 0");
    // calculate number of page blocks in the index
    // size_t inner_node_size = std::pow(2.0, num_layers_) - 1;

    // size_t offset = construct_page_block(0, 0, inner_node_size);

    // while (offset < inner_node_size) {
    //   offset = construct_page_block(offset, inner_node_size);
    // }
    construct_page_block(0, 0, this->size_ - 1);

    size_t step = (this->size_ - 1) / 256;

    // for (size_t i = 0; i < 256; ++i) {
    //   construct_page_block(0 + 256 * i, 0 + step * i, 0 + step * (i + 1));
    // }

  }

  // we assume pagesize = 4 KB.
  void construct_page_block(const size_t current_pos, const size_t lhs_offset, const size_t rhs_offset) {

    construct_cacheline_block(current_pos, lhs_offset, rhs_offset);

    size_t step = (rhs_offset - lhs_offset) / 16;

    for (size_t i = 0; i < 16; ++i) {
      construct_cacheline_block(current_pos + 16 * (i + 1), lhs_offset + step * i, lhs_offset + step * (i + 1));
    }

    // return current_pos + 256;
  }
 
  // we only support the case for simd key capacity = 3.
  // in this case, the number of simd blocks in each cacheline block is 5.
  void construct_cacheline_block(const size_t current_pos, const size_t lhs_offset, const size_t rhs_offset) {

    // level 0
    construct_simd_block(current_pos, lhs_offset, rhs_offset);

    size_t step = (rhs_offset - lhs_offset) / 4;

    // level 1
    for (size_t i = 0; i < 4; ++i) {
      construct_simd_block(current_pos + 3 * (i + 1), lhs_offset + step * i, lhs_offset + step * (i + 1));
    }
    
    // CPU fetches data in cacheline size. 
    // return current_pos + 16;
  }

  // we only support the case for simd key capacity = 3.
  void construct_simd_block(const size_t current_pos, const size_t lhs_offset, const size_t rhs_offset) {

    size_t med = median(lhs_offset, rhs_offset);
    inner_nodes_[current_pos] = this->container_[med].key_;
    inner_nodes_[current_pos + 1] = this->container_[median(lhs_offset, med)].key_;
    inner_nodes_[current_pos + 2] = this->container_[median(med, rhs_offset)].key_;

  }

  // find in leaf nodes, simple binary search [incl., excl.)
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
    // return std::pair<int, int>(0, this->size_);

    if (num_layers_ == 0) { return std::pair<int, int>(0, this->size_); }

    // search in cacheline block
    int begin_offset = 0;
    int end_offset = this->size_ - 1;
    
    int mid_offset = median(begin_offset, end_offset);
    int lhs_mid_offset = median(begin_offset, mid_offset);
    int rhs_mid_offset = median(mid_offset, end_offset);

    // search in level 0 simd block
    int part = -1;
    if (key > inner_nodes_[2]) { 
      part = 3;
      // return std::pair<int, int>(rhs_mid_offset + 1, end_offset + 1); 
    }
    else if (key > inner_nodes_[0]) { 
      part = 2;
      // return std::pair<int, int>(mid_offset + 1, rhs_mid_offset + 1); 
    } 
    else if (key > inner_nodes_[1]) { 
      part = 1;
      // return std::pair<int, int>(lhs_mid_offset + 1, mid_offset + 1); 
    } 
    else {
      part = 0;
      // return std::pair<int, int>(0, lhs_mid_offset + 1);
    }
    size_t step = (end_offset - begin_offset) / 16;
    // search in level 1 simd block
    int current_pos = (part + 1) * 3;
    std::cout << inner_nodes_[current_pos + 2] << " " << inner_nodes_[current_pos + 0] << " " << inner_nodes_[current_pos + 1] << std::endl;
    if (key > inner_nodes_[current_pos + 2]) {
      part = part * 4 + 3;
    }
    else if (key > inner_nodes_[current_pos + 0]) {
      part = part * 4 + 2;
    }
    else if (key > inner_nodes_[current_pos + 1]) {
      part = part * 4 + 1;
    }
    else {
      part = part * 4 + 0;
    }
    std::cout << "key = " << key << " " << 0 + step * part << " " << 0 + step * (part + 1) + 1 << " " << part << std::endl;
    // std::cout << "part = " << part << std::endl;
    return std::pair<int, int>(0 + step * part, 0 + step * (part + 1) + 2);
    

    // size_t base_pos = 1;
    // size_t next_layer = 1;

    // if (key < inner_nodes_[0]) {
    //   return find_inner_layers_internal(key, begin_offset, mid_offset - 1, base_pos, 0, next_layer);
    // } else {
    //   return find_inner_layers_internal(key, mid_offset + 1, end_offset, base_pos, 1, next_layer);
    // }

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

  size_t page_key_capacity_;
  size_t page_depth_;

};

}