#pragma once

#include "static_index/interpolation_index.h"
#include "static_index/interpolation_index_v1.h"
#include "static_index/interpolation_index_v2.h"
#include "static_index/fast_index.h"

#include "singlethread_dynamic_index/btree_index.h"
#include "singlethread_dynamic_index/stx_btree_index.h"
#include "singlethread_dynamic_index/art_tree_index.h"

#include "multithread_dynamic_index/libcuckoo_index.h"
#include "multithread_dynamic_index/art_tree_index.h"
#include "multithread_dynamic_index/bw_tree_index.h"

enum class IndexType {
  // static indexes
  StaticInterpolationIndexType = 0,
  StaticInterpolationIndexTypeV1,
  StaticInterpolationIndexTypeV2,
  
  // singlethread dynamic indexes
  SinglethreadDynamicBtreeIndexType = 3,
  SinglethreadDynamicStxBtreeIndexType,
  SinglethreadDynamicArtTreeIndexType,
  
  // multithread dynamic indexes
  MultithreadDynamicLibcuckooIndexType = 6,
  MultithreadDynamicArtTreeIndexType,
  MultithreadDynamicBwTreeIndexType,
};

static std::string get_index_name(const IndexType index_type) {
  if (index_type == IndexType::StaticInterpolationIndexType) {
    return "interpolation_index";
  } else if (index_type == IndexType::StaticInterpolationIndexTypeV1) {
    return "interpolation_index_v1";
  } else if (index_type == IndexType::SinglethreadDynamicBtreeIndexType) {
    return "singlethread dynamic - btree index";
  } else if (index_type == IndexType::SinglethreadDynamicStxBtreeIndexType) {
    return "singlethread dynamic - stx btree index";
  } else if (index_type == IndexType::SinglethreadDynamicArtTreeIndexType) {
    return "singlethread dynamic - art tree index";
  } else if (index_type == IndexType::MultithreadDynamicLibcuckooIndexType) {
    return "multithread dynamic - libcuckoo index";
  } else if (index_type == IndexType::MultithreadDynamicArtTreeIndexType) {
    return "multithread dynamic - art tree index";
  } else if (index_type == IndexType::MultithreadDynamicBwTreeIndexType) {
    return "multithread dynamic - bw btree index";
  }
}

template<typename KeyT>
static BaseIndex<KeyT>* create_index(const IndexType index_type, DataTable<KeyT, uint64_t> *table_ptr = nullptr, const size_t segment_count = 0) {
  if (index_type == IndexType::SinglethreadDynamicBtreeIndexType) {

    return new InterpolationIndex<KeyT>();
  
  } else if (index_type == IndexType::StaticInterpolationIndexType) {

    return new InterpolationIndex<KeyT>();
  
  } else if (index_type == IndexType::StaticInterpolationIndexTypeV1) {

    return new InterpolationIndexV1<KeyT>();

  } else if (index_type == IndexType::StaticInterpolationIndexTypeV2) {

    assert(segment_count != 0);
    
    return new InterpolationIndexV2<KeyT>(segment_count);

  } else if (index_type == IndexType::SinglethreadDynamicBtreeIndexType) {

    // return new singlethread_dynamic_index::LibcuckooIndex<KeyT>();

  } else if (index_type == IndexType::SinglethreadDynamicStxBtreeIndexType) {

    return new singlethread_dynamic_index::StxBtreeIndex<KeyT>();

  } else if (index_type == IndexType::SinglethreadDynamicArtTreeIndexType) {

    return new singlethread_dynamic_index::ArtTreeIndex<KeyT>();

  } else if (index_type == IndexType::MultithreadDynamicLibcuckooIndexType) {

    return new multithread_dynamic_index::LibcuckooIndex<KeyT>();

  } else if (index_type == IndexType::MultithreadDynamicArtTreeIndexType) {

    return new multithread_dynamic_index::ArtTreeIndex<KeyT, Uint64>(table_ptr);

  } else if (index_type == IndexType::MultithreadDynamicBwTreeIndexType) {

    return new multithread_dynamic_index::BwTreeIndex<KeyT>();

  } else {
    assert(false);
    return nullptr;
  }
}