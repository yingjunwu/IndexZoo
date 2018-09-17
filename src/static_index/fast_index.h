#pragma once

#include <vector>

#include <emmintrin.h>

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

  const size_t KEY_SIZE = 4; // FAST only support 4-byte keys.
  const size_t SIMD_KEY_CAPACITY = 3; 
  const size_t CACHELINE_KEY_CAPACITY = 15;
  const size_t CACHELINE_DEPTH = 4;


public:
  FastIndex(DataTable<KeyT, ValueT> *table_ptr, const size_t num_layers)
    : BaseStaticIndex<KeyT, ValueT>(table_ptr)
    , num_layers_(num_layers) {

    ASSERT(sizeof(KeyT) == KEY_SIZE, "only support 4-byte keys");
  }

  virtual ~FastIndex() {
    if (num_layers_ != 0) {
      delete[] inner_nodes_;
      inner_nodes_ = nullptr;

      delete num_cachelines_;
      num_cachelines_ = nullptr;
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

    size_t inner_node_size = std::pow(2.0, num_layers_) - 1;

    ASSERT(inner_node_size < this->size_, "exceed maximum layers");

    cacheline_levels_ = num_layers_ / CACHELINE_DEPTH;
    num_cachelines_ = new size_t[cacheline_levels_ + 1];
    for (size_t i = 0; i < cacheline_levels_ + 1; ++i) {
      num_cachelines_[i] = std::pow(16, i);
    }
    
    lhs_offset_ = 0;
    rhs_offset_ = this->size_ - 1 - this->size_ % num_cachelines_[cacheline_levels_];

    last_level_step_ = (rhs_offset_ - lhs_offset_ + 1) / num_cachelines_[cacheline_levels_];

    key_min_ = this->container_[0].key_;
    key_max_ = this->container_[this->size_ - 1].key_;

    if (num_layers_ != 0) {

      size_t num_cachelines = inner_node_size / CACHELINE_KEY_CAPACITY;
      inner_size_ = num_cachelines * CACHELINE_SIZE / sizeof(KeyT);
      inner_nodes_ = new KeyT[inner_size_];
      memset(inner_nodes_, 0, sizeof(KeyT) * inner_size_);

      construct_inner_layers();
    } else {
      inner_nodes_ = nullptr;
    }
  }

  virtual void print() const final {
    if (inner_nodes_ != nullptr) {
      for (size_t i = 0; i < inner_size_; ++i) {
        std::cout << inner_nodes_[i] << " ";
      }
      std::cout << std::endl;
    }
  }

private:

  void construct_inner_layers() {
    ASSERT(num_layers_ != 0, "number of layers cannot be 0");

    // cacheline level 0
    size_t current_pos = 0;
    construct_cacheline_block(current_pos, lhs_offset_, rhs_offset_);
    current_pos += 16; // cacheline size

    // cacheline level i
    for (size_t i = 1; i < cacheline_levels_; ++i) {
      size_t num_cachelines = std::pow(16, i);
      size_t step = (rhs_offset_ - lhs_offset_ + 1) / num_cachelines;

      for (size_t j = 0; j < num_cachelines; ++j) {
        construct_cacheline_block(current_pos, step * j, step * (j + 1) - 1);
        current_pos += 16;
      }
    }
  }
 
  // we only support the case for simd key capacity = 3.
  // in this case, the number of simd blocks in each cacheline block is 5.
  void construct_cacheline_block(const size_t current_pos, const size_t lhs_offset, const size_t rhs_offset) {

    // simd level 0
    construct_simd_block(current_pos, lhs_offset, rhs_offset);

    // simd level 1
    size_t step = (rhs_offset - lhs_offset + 1) / 4;
    for (size_t i = 0; i < 4; ++i) {
      construct_simd_block(current_pos + 3 * (i + 1), lhs_offset + step * i, lhs_offset + step * (i + 1) - 1);
    }
  }

  // we only support the case for simd key capacity = 3.
  void construct_simd_block(const size_t current_pos, const size_t lhs_offset, const size_t rhs_offset) {

    size_t step = (rhs_offset - lhs_offset + 1) / 4;

    inner_nodes_[current_pos + 0] = this->container_[lhs_offset + 2 * step - 1].key_;
    inner_nodes_[current_pos + 1] = this->container_[lhs_offset + 1 * step - 1].key_;
    inner_nodes_[current_pos + 2] = this->container_[lhs_offset + 3 * step - 1].key_;

  }

  // find in inner nodes
  std::pair<int, int> find_inner_layers(const KeyT &key) {

    if (num_layers_ == 0) { return std::pair<int, int>(0, this->size_ - 1); }

    // cacheline level 0
    size_t current_pos = 0;
    size_t branch_id = lookup_cacheline_block(key, current_pos);
    current_pos += 16; // beginning position in next level
    
    size_t num_cachelines = std::pow(16, 1); // number of cachelines in next level
    
    for (size_t i = 1; i < cacheline_levels_; ++i) {
      
      size_t new_branch_id = lookup_cacheline_block(key, current_pos + branch_id * 16);
      
      branch_id = branch_id * 16 + new_branch_id;
      current_pos += 16 * num_cachelines; // beginning position in next level

      num_cachelines = std::pow(16, (i + 1)); // number of cachelines in next level
    }
    
    if (branch_id < num_cachelines - 1) {

      return std::pair<int, int>(branch_id * last_level_step_, (branch_id + 1) * last_level_step_ - 1);
    } else {

      return std::pair<int, int>(branch_id * last_level_step_, this->size_ - 1);
    }
    
  }

  // search in cacheline block
  size_t lookup_cacheline_block(const KeyT &key, const size_t current_pos) {

    size_t branch_id = lookup_simd_block(key, current_pos);
    
    size_t new_pos = current_pos + 3 * (branch_id + 1);

    size_t new_branch_id = lookup_simd_block(key, new_pos); 

    return branch_id * 4 + new_branch_id;
  }

  // search in simd block
  size_t lookup_simd_block(const KeyT &key, const size_t current_pos) {

    __m128i xmm_key_q =_mm_set1_epi32(key);
    __m128i xmm_tree = _mm_loadu_si128((__m128i*)(inner_nodes_ + current_pos));
    __m128i xmm_mask = _mm_cmpgt_epi32(xmm_key_q, xmm_tree);
    unsigned index = _mm_movemask_ps(_mm_castsi128_ps(xmm_mask));

    static unsigned table[8] = {0, 9, 1, 2, 9, 9, 9, 3}; // 9 stands for impossible
    size_t branch_id = table[(index&7)];

    return branch_id;
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

  size_t lhs_offset_;
  size_t rhs_offset_;
  size_t last_level_step_;
  size_t cacheline_levels_;
  size_t *num_cachelines_;


};

}