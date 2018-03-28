#pragma once

#include <cassert>

#include "static_index/interpolation_index.h"
#include "static_index/binary_index.h"
#include "static_index/kary_index.h"
#include "static_index/fast_index.h"

#include "dynamic_index/singlethread/stx_btree_index.h"
#include "dynamic_index/singlethread/art_tree_index.h"
#include "dynamic_index/singlethread/skiplist_index.h"
#include "dynamic_index/singlethread/btree_index.h"

#include "dynamic_index/multithread/libcuckoo_index.h"
#include "dynamic_index/multithread/art_tree_index.h"
#include "dynamic_index/multithread/bw_tree_index.h"
#include "dynamic_index/multithread/masstree_index.h"


enum class IndexType {
  // static indexes
  S_Interpolation = 0,
  S_Binary, 
  S_KAry, 
  S_Fast,

  // dynamic indexes - singlethread
  D_ST_StxBtree = 10,
  D_ST_ArtTree,
  D_ST_Skiplist,
  D_ST_Btree,
  
  // dynamic indexes - multithread
  D_MT_Libcuckoo = 20,
  D_MT_ArtTree,
  D_MT_BwTree,
  D_MT_Masstree,

};


static std::string get_index_name(const IndexType index_type) {
  if (index_type == IndexType::S_Interpolation) {
    return "static - interpolation index";
  } else if (index_type == IndexType::S_Binary) {
    return "static - binary index";
  } else if (index_type == IndexType::S_KAry) {
    return "static - k-ary index";
  } else if (index_type == IndexType::S_Fast) {
    return "static - fast index";
  } else if (index_type == IndexType::D_ST_StxBtree) {
    return "dynamic - singlethread - stx-btree index";
  } else if (index_type == IndexType::D_ST_ArtTree) {
    return "dynamic - singlethread - art-tree index";
  } else if (index_type == IndexType::D_ST_Skiplist) {
    return "dynamic - singlethread - skiplist index";
  } else if (index_type == IndexType::D_MT_Libcuckoo) {
    return "dynamic - multithread - libcuckoo index";
  } else if (index_type == IndexType::D_MT_ArtTree) {
    return "dynamic - multithread - art-tree index";
  } else if (index_type == IndexType::D_MT_BwTree) {
    return "dynamic - multithread - bw-tree index";
  } else if (index_type == IndexType::D_MT_Masstree) {
    return "dynamic - multithread - masstree index";
  } else {
    ASSERT(false, "invalid index type");
    return "";
  }
}

static const size_t INVALID_INDEX_PARAM = std::numeric_limits<size_t>::max();

static void validate_index_params(const IndexType index_type, const size_t index_param_1, const size_t index_param_2) {
  if (index_type == IndexType::S_Interpolation) {

    if (index_param_1 == 0) {
      std::cerr << "expected index type: static - interpolation index" << std::endl;
      std::cerr << "error: number of segments cannot be 0!" << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    std::cout << "index type: static - interpolation index" << std::endl;
    std::cout << "number of segments: " << index_param_1 << std::endl;

  } else if (index_type == IndexType::S_Binary) {
    
    std::cout << "index type: static - binary index" << std::endl;
    std::cout << "number of layers: " << index_param_1 << std::endl;

  } else if (index_type == IndexType::S_KAry) {
    
    if (index_param_2 < 2) {
      std::cerr << "expected index type: static - k-ary index" << std::endl;
      std::cerr << "error: k must be larger than or equal to 2!" << std::endl;
      exit(EXIT_FAILURE);
      return;
    }

    std::cout << "index type: static - k-ary index" << std::endl;
    std::cout << "number of layers: " << index_param_1 << std::endl;
    std::cout << "number of arys: " << index_param_2 << std::endl;

  } else if (index_type == IndexType::S_Fast) {
    
    std::cout << "index type: static - fast index" << std::endl;

  } else {
    
    std::cout << "index type: " << get_index_name(index_type) << std::endl;

    return;
  }
}

template<typename KeyT, typename ValueT>
static BaseIndex<KeyT, ValueT>* create_index(const IndexType index_type, DataTable<KeyT, uint64_t> *table_ptr, const size_t index_param_1 = INVALID_INDEX_PARAM, const size_t index_param_2 = INVALID_INDEX_PARAM) {
  if (index_type == IndexType::S_Interpolation) {

    ASSERT(index_param_1 != 0, "number of segments cannot be 0");

    return new static_index::InterpolationIndex<KeyT, ValueT>(table_ptr, index_param_1);
  
  } else if (index_type == IndexType::S_Binary) {

    return new static_index::BinaryIndex<KeyT, ValueT>(table_ptr, index_param_1);

  } else if (index_type == IndexType::S_KAry) {

    ASSERT(index_param_2 >= 2, "k must be larger than or equal to 2");

    return new static_index::KAryIndex<KeyT, ValueT>(table_ptr, index_param_1, index_param_2);

  } else if (index_type == IndexType::S_Fast) {

    return new static_index::FastIndex<KeyT, ValueT>(table_ptr);

  } else if (index_type == IndexType::D_ST_StxBtree) {

    return new dynamic_index::singlethread::StxBtreeIndex<KeyT, ValueT>(table_ptr);

  } else if (index_type == IndexType::D_ST_ArtTree) {

    return new dynamic_index::singlethread::ArtTreeIndex<KeyT, ValueT>(table_ptr);

  } else if (index_type == IndexType::D_ST_Skiplist) {

    return new dynamic_index::singlethread::SkiplistIndex<KeyT, ValueT>(table_ptr);

  } else if (index_type == IndexType::D_MT_Libcuckoo) {

    return new dynamic_index::multithread::LibcuckooIndex<KeyT, ValueT>(table_ptr);

  } else if (index_type == IndexType::D_MT_ArtTree) {

    return new dynamic_index::multithread::ArtTreeIndex<KeyT, ValueT>(table_ptr);

  } else if (index_type == IndexType::D_MT_BwTree) {

    return new dynamic_index::multithread::BwTreeIndex<KeyT, ValueT>(table_ptr);

  } else if (index_type == IndexType::D_MT_Masstree) {

    return new dynamic_index::multithread::MasstreeIndex<KeyT, ValueT>(table_ptr);

  } else {

    ASSERT(false, "unsupported index type");
    return nullptr;
  }
}
