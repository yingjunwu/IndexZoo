#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "harness.h"
#include "fast_random.h"
#include "time_measurer.h"

#include "data_table.h"

#include "index_all.h"


class DynamicIndexTest : public IndexZooTest {};

template<typename KeyT, typename ValueT>
void test_dynamic_index_unique_key(const IndexType index_type) {

  size_t n = 10000;

  std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
    new DataTable<KeyT, ValueT>());
  std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
    create_index<KeyT, ValueT>(index_type, data_table.get()));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  std::unordered_map<KeyT, std::pair<Uint64, ValueT>> validation_set;
  
  // insert
  for (size_t i = 0; i < n; ++i) {

    KeyT key = i;
    ValueT value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    
    validation_set.insert(
      std::pair<KeyT, std::pair<Uint64, ValueT>>(
        key, std::pair<Uint64, ValueT>(offset.raw_data(), value)));

    data_index->insert(key, offset.raw_data());
  }

  // find
  for (size_t i = 0; i < n; ++i) {
    KeyT key = i;

    std::vector<Uint64> offsets;

    data_index->find(key, offsets);

    EXPECT_EQ(offsets.size(), 1);

    ValueT *value = data_table->get_tuple_value(offsets.at(0));

    EXPECT_EQ(offsets.at(0), validation_set.find(key)->second.first);

    EXPECT_EQ(*value, validation_set.find(key)->second.second);
  }
}

TEST_F(DynamicIndexTest, UniqueKeyTest) {

  std::vector<IndexType> index_types {

    // dynamic indexes - singlethread
    IndexType::D_ST_StxBtree,
    IndexType::D_ST_ArtTree,
    // IndexType::D_ST_Btree, // un-implemented
    
    // dynamic indexes - multithread
    IndexType::D_MT_Libcuckoo,
    IndexType::D_MT_ArtTree,
    IndexType::D_MT_BwTree,
    IndexType::D_MT_Masstree,
  };

  for (auto index_type : index_types) {
    // key type is set to uint16_t
    test_dynamic_index_unique_key<uint16_t, uint64_t>(index_type);
    
    // key type is set to uint32_t
    test_dynamic_index_unique_key<uint32_t, uint64_t>(index_type);
    
    // key type is set to uint64_t
    test_dynamic_index_unique_key<uint64_t, uint64_t>(index_type);
  }

}


template<typename KeyT, typename ValueT>
void test_dynamic_index_non_unique_key(const IndexType index_type) {

  size_t n = 10000;
  size_t m = 1000;
  
  FastRandom rand_gen(0);

  std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
    new DataTable<KeyT, ValueT>());
  std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
    create_index<KeyT, ValueT>(index_type, data_table.get()));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  std::unordered_map<KeyT, std::unordered_map<Uint64, ValueT>> validation_set;
  
  // insert
  for (size_t i = 0; i < n; ++i) {

    KeyT key = rand_gen.next<KeyT>() % m;
    ValueT value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    
    validation_set[key][offset.raw_data()] = value;

    data_index->insert(key, offset.raw_data());
  }

  // find
  for (auto entry : validation_set) {
    KeyT key = entry.first;

    std::vector<Uint64> offsets;

    data_index->find(key, offsets);

    EXPECT_EQ(offsets.size(), entry.second.size());

    for (auto offset : offsets) {
      ValueT *value = data_table->get_tuple_value(offset);

      EXPECT_NE(entry.second.end(), entry.second.find(offset));

      EXPECT_EQ(*value, entry.second.at(offset));
    }
  }
}


TEST_F(DynamicIndexTest, NonUniqueKeyTest) {

  std::vector<IndexType> index_types {

    // dynamic indexes - singlethread
    IndexType::D_ST_StxBtree,
    // IndexType::D_ST_ArtTree, // do not support non-unique keys
    // IndexType::D_ST_Btree,
    
    // dynamic indexes - multithread
    IndexType::D_MT_Libcuckoo,
    IndexType::D_MT_ArtTree,
    IndexType::D_MT_BwTree,
    // IndexType::D_MT_Masstree, // do not support non-unique keys
  };

  for (auto index_type : index_types) {

    // key type is set to uint16_t
    test_dynamic_index_non_unique_key<uint16_t, uint64_t>(index_type);

    // key type is set to uint32_t
    test_dynamic_index_non_unique_key<uint32_t, uint64_t>(index_type);

    // key type is set to uint64_t
    test_dynamic_index_non_unique_key<uint64_t, uint64_t>(index_type);
  }
}

// TEST_F(DynamicIndexTest, RangeFindTest) {

  // std::vector<IndexType> index_types {

  //   // dynamic indexes - singlethread
  //   IndexType::D_ST_StxBtree,
  //   IndexType::D_ST_ArtTree,
  //   // IndexType::D_ST_Btree,
    
  //   // dynamic indexes - multithread
  //   IndexType::D_MT_Libcuckoo,
  //   IndexType::D_MT_ArtTree,
  //   IndexType::D_MT_BwTree,
  //   // IndexType::D_MT_Masstree,
  // };

//   for (auto index_type : index_types) {

//     std::unique_ptr<DataTable<uint64_t, uint64_t>> data_table(
//       new DataTable<uint64_t, uint64_t>());
//     std::unique_ptr<BaseDynamicIndex<uint64_t>> data_index(
//       create_index<uint64_t>(index_type, data_table.get()));

//     data_index->prepare_threads(1);
//     data_index->register_thread(0);

//     std::multimap<uint64_t, std::pair<Uint64, uint64_t>> validation_set;
    
//     // insert
//     for (size_t i = 0; i < 10000; ++i) {

//       uint64_t key = i;
//       uint64_t value = i + 2048;
      
//       OffsetT offset = data_table->insert_tuple(key, value);
      
//       validation_set.insert(
//         std::pair<uint64_t, std::pair<Uint64, uint64_t>>(
//           key, std::pair<Uint64, uint64_t>(offset.raw_data(), value)));

//       data_index->insert(key, offset.raw_data());
//     }

//     // find
//     for (size_t i = 0; i < 100; ++i) {
//       uint64_t key = i;

//       std::vector<Uint64> offsets;

//       data_index->find(key, offsets);

//       EXPECT_EQ(offsets.size(), 1);

//       uint64_t *value = data_table->get_tuple_value(offsets.at(0));

//       EXPECT_EQ(offsets.at(0), validation_set.find(key)->second.first);

//       EXPECT_EQ(*value, validation_set.find(key)->second.second);
//     }
//   }

// }




