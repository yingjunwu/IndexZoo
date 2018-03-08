#pragma once

#include "static_index/interpolation_index.h"
#include "static_index/interpolation_index_v1.h"
#include "static_index/interpolation_index_v2.h"
#include "static_index/fast_index.h"

#include "dynamic_index_st/stx_btree_index.h"
#include "dynamic_index_st/btree_index.h"

enum class IndexType {
  InterpolationIndexType = 0,
  InterpolationIndexTypeV1,
  InterpolationIndexTypeV2,
  StxBtreeIndexType,
  BtreeIndexType,
};

static std::string get_index_name(const IndexType index_type) {
  if (index_type == IndexType::StxBtreeIndexType) {
    return "stx_btree_index";
  } else if (index_type == IndexType::BtreeIndexType) {
    return "btree_index";
  } else if (index_type == IndexType::InterpolationIndexType) {
    return "interpolation_index";
  } else if (index_type == IndexType::InterpolationIndexTypeV1) {
    return "interpolation_index_v1";
  } else if (index_type == IndexType::InterpolationIndexTypeV2) {
    return "interpolation_index_v2";
  }
}


template<typename KeyT>
static BaseIndex<KeyT>* create_index(const IndexType index_type, const size_t segment_count = 0) {
  if (index_type == IndexType::StxBtreeIndexType) {

    return new StxBtreeIndex<KeyT>();

  } else if (index_type == IndexType::BtreeIndexType) {

    return new InterpolationIndex<KeyT>();
  
  } else if (index_type == IndexType::InterpolationIndexType) {

    return new InterpolationIndex<KeyT>();
  
  } else if (index_type == IndexType::InterpolationIndexTypeV1) {

    return new InterpolationIndexV1<KeyT>();

  } else if (index_type == IndexType::InterpolationIndexTypeV2) {

    assert(segment_count != 0);
    
    return new InterpolationIndexV2<KeyT>(segment_count);

  } else {
    assert(false);
    return nullptr;
  }
}