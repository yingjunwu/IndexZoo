#pragma once

#include <cassert>

#include "static_index/interpolation_index.h"
#include "static_index/binary_index.h"
#include "static_index/binary_search_index.h"

#include "dynamic_index/singlethread/btree_index.h"
#include "dynamic_index/singlethread/stx_btree_index.h"
#include "dynamic_index/singlethread/art_tree_index.h"

#include "dynamic_index/multithread/libcuckoo_index.h"
#include "dynamic_index/multithread/art_tree_index.h"
#include "dynamic_index/multithread/bw_tree_index.h"
#include "dynamic_index/multithread/masstree_index.h"


// static indexes
enum class StaticIndexType {
  InterpolationIndexType = 0,
  BinaryIndexType, 
  BinarySearchIndexType, 
  KAryIndexType, 
  // FastIndexType,
};

// dynamic indexes
enum class DynamicIndexType {
  // singlethread
  SinglethreadBtreeIndexType = 0,
  SinglethreadStxBtreeIndexType,
  SinglethreadArtTreeIndexType,
  
  // multithread
  MultithreadLibcuckooIndexType = 3,
  MultithreadArtTreeIndexType,
  MultithreadBwTreeIndexType,
  MultithreadMasstreeIndexType,
};

static std::string get_static_index_name(const StaticIndexType index_type) {
  if (index_type == StaticIndexType::InterpolationIndexType) {
    return "static - interpolation index";
  } else if (index_type == StaticIndexType::BinaryIndexType) {
    return "static - binary index";
  } else if (index_type == StaticIndexType::BinarySearchIndexType) {
    return "static - binary search index";
  } else if (index_type == StaticIndexType::KAryIndexType) {
    return "static - k-ary index";
  } else {
    assert(false);
    return "";
  }
}

static std::string get_dynamic_index_name(const DynamicIndexType index_type) {
  if (index_type == DynamicIndexType::SinglethreadBtreeIndexType) {
    return "dynamic - singlethread - btree index";
  } else if (index_type == DynamicIndexType::SinglethreadStxBtreeIndexType) {
    return "dynamic - singlethread - stx-btree index";
  } else if (index_type == DynamicIndexType::SinglethreadArtTreeIndexType) {
    return "dynamic - singlethread - art-tree index";
  } else if (index_type == DynamicIndexType::MultithreadLibcuckooIndexType) {
    return "dynamic - multithread - libcuckoo index";
  } else if (index_type == DynamicIndexType::MultithreadArtTreeIndexType) {
    return "dynamic - multithread - art-tree index";
  } else if (index_type == DynamicIndexType::MultithreadBwTreeIndexType) {
    return "dynamic - multithread - bw-tree index";
  } else if (index_type == DynamicIndexType::MultithreadMasstreeIndexType) {
    return "dynamic - multithread - masstree index";
  } else {
    assert(false);
    return "";
  }
}

template<typename KeyT, typename ValueT>
static BaseStaticIndex<KeyT, ValueT>* create_static_index(const StaticIndexType index_type, DataTable<KeyT, uint64_t> *table_ptr, const size_t segment_count = 1) {
  if (index_type == StaticIndexType::InterpolationIndexType) {

    assert(segment_count != 0);

    return new static_index::InterpolationIndex<KeyT, Uint64>(table_ptr, segment_count);
  
  } else if (index_type == StaticIndexType::BinaryIndexType) {

    return new static_index::BinaryIndex<KeyT, Uint64>(table_ptr);

  } else if (index_type == StaticIndexType::BinarySearchIndexType) {

    return new static_index::BinarySearchIndex<KeyT, Uint64>(table_ptr);

  } else if (index_type == StaticIndexType::KAryIndexType) {

    return nullptr;

  } else {

    ASSERT(false, "invalid static index type");
    return nullptr;
  }
}


template<typename KeyT>
static BaseDynamicIndex<KeyT>* create_dynamic_index(const DynamicIndexType index_type, DataTable<KeyT, uint64_t> *table_ptr) {
  if (index_type == DynamicIndexType::SinglethreadBtreeIndexType) {

    // return new dynamic_index::singlethread::LibcuckooIndex<KeyT>();
    return nullptr;

  } else if (index_type == DynamicIndexType::SinglethreadStxBtreeIndexType) {

    return new dynamic_index::singlethread::StxBtreeIndex<KeyT>();

  } else if (index_type == DynamicIndexType::SinglethreadArtTreeIndexType) {

    return new dynamic_index::singlethread::ArtTreeIndex<KeyT>();

  } else if (index_type == DynamicIndexType::MultithreadLibcuckooIndexType) {

    return new dynamic_index::multithread::LibcuckooIndex<KeyT>();

  } else if (index_type == DynamicIndexType::MultithreadArtTreeIndexType) {

    return new dynamic_index::multithread::ArtTreeIndex<KeyT, Uint64>(table_ptr);

  } else if (index_type == DynamicIndexType::MultithreadBwTreeIndexType) {

    return new dynamic_index::multithread::BwTreeIndex<KeyT>();

  } else if (index_type == DynamicIndexType::MultithreadMasstreeIndexType) {

    return new dynamic_index::multithread::MasstreeIndex<KeyT>();

  } else {

    ASSERT(false, "invalid dynamic index type");
    return nullptr;
  }
}
