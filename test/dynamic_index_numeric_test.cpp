#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "harness.h"
#include "fast_random.h"
#include "time_measurer.h"

#include "data_table.h"

#include "index_all.h"


class DynamicIndexNumericTest : public IndexZooTest {};

template<typename KeyT, typename ValueT>
void test_dynamic_index_numeric_unique_key_find(const IndexType index_type) {

  size_t n = 10000;

  std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
    new DataTable<KeyT, ValueT>());
  std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
    create_index<KeyT, ValueT>(index_type, data_table.get()));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  std::unordered_map<KeyT, std::pair<Uint64, ValueT>> validation_set;
    
  FastRandom rand;

  // insert
  for (size_t i = 0; i < n; ++i) {

    KeyT key = rand.next<KeyT>();
    // KeyT key = i;
    ValueT value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    
    validation_set.insert(
      std::pair<KeyT, std::pair<Uint64, ValueT>>(
        key, std::pair<Uint64, ValueT>(offset.raw_data(), value)));

    data_index->insert(key, offset.raw_data());
  }

  // find
  for (auto entry : validation_set) {
    KeyT key = entry.first;

    std::vector<Uint64> offsets;

    data_index->find(key, offsets);

    EXPECT_EQ(offsets.size(), 1);

    ValueT *value = data_table->get_tuple_value(offsets.at(0));

    EXPECT_EQ(offsets.at(0), entry.second.first);

    EXPECT_EQ(*value, entry.second.second);
  }
}

TEST_F(DynamicIndexNumericTest, UniqueKeyFindTest) {

  std::vector<IndexType> index_types {

    // dynamic indexes - singlethread
    // IndexType::D_ST_StxBtree,
    IndexType::D_ST_ArtTree,
    // IndexType::D_ST_Btree, // unimplemented
    
    // dynamic indexes - multithread
    // IndexType::D_MT_Libcuckoo,
    // IndexType::D_MT_ArtTree,
    // IndexType::D_MT_BwTree,
    // IndexType::D_MT_Masstree,
  };

  for (auto index_type : index_types) {
    // key type is set to uint16_t
    // test_dynamic_index_numeric_unique_key_find<uint16_t, uint64_t>(index_type);
    
    // key type is set to uint32_t
    // test_dynamic_index_numeric_unique_key_find<uint32_t, uint64_t>(index_type);
    
    // key type is set to uint64_t
    test_dynamic_index_numeric_unique_key_find<uint64_t, uint64_t>(index_type);
  }

}


// template<typename KeyT, typename ValueT>
// void test_dynamic_index_numeric_non_unique_key_find(const IndexType index_type) {

//   size_t n = 10000;
//   size_t m = 1000;
  
//   FastRandom rand_gen(0);

//   std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
//     new DataTable<KeyT, ValueT>());
//   std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
//     create_index<KeyT, ValueT>(index_type, data_table.get()));

//   data_index->prepare_threads(1);
//   data_index->register_thread(0);

//   std::unordered_map<KeyT, std::unordered_map<Uint64, ValueT>> validation_set;
  
//   // insert
//   for (size_t i = 0; i < n; ++i) {

//     KeyT key = rand_gen.next<KeyT>() % m;
//     ValueT value = i + 2048;
    
//     OffsetT offset = data_table->insert_tuple(key, value);
    
//     validation_set[key][offset.raw_data()] = value;

//     data_index->insert(key, offset.raw_data());
//   }

//   // find
//   for (auto entry : validation_set) {
//     KeyT key = entry.first;

//     std::vector<Uint64> offsets;

//     data_index->find(key, offsets);

//     EXPECT_EQ(offsets.size(), entry.second.size());

//     for (auto offset : offsets) {
//       ValueT *value = data_table->get_tuple_value(offset);

//       EXPECT_NE(entry.second.end(), entry.second.find(offset));

//       EXPECT_EQ(*value, entry.second.at(offset));
//     }
//   }
// }


// TEST_F(DynamicIndexNumericTest, NonUniqueKeyFindTest) {

//   std::vector<IndexType> index_types {

//     // dynamic indexes - singlethread
//     IndexType::D_ST_StxBtree,
//     // IndexType::D_ST_ArtTree, // do not support non-unique keys
//     // IndexType::D_ST_Btree, // unimplemented
    
//     // dynamic indexes - multithread
//     IndexType::D_MT_Libcuckoo,
//     IndexType::D_MT_ArtTree,
//     IndexType::D_MT_BwTree,
//     // IndexType::D_MT_Masstree, // do not support non-unique keys
//   };

//   for (auto index_type : index_types) {

//     // key type is set to uint16_t
//     // test_dynamic_index_numeric_non_unique_key_find<uint16_t, uint64_t>(index_type);

//     // key type is set to uint32_t
//     test_dynamic_index_numeric_non_unique_key_find<uint32_t, uint64_t>(index_type);

//     // key type is set to uint64_t
//     test_dynamic_index_numeric_non_unique_key_find<uint64_t, uint64_t>(index_type);
//   }
// }


// template<typename KeyT, typename ValueT>
// void test_dynamic_index_numeric_unique_key_find_range(const IndexType index_type) {

//   size_t n = 10000;

//   std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
//     new DataTable<KeyT, ValueT>());
//   std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
//     create_index<KeyT, ValueT>(index_type, data_table.get()));

//   data_index->prepare_threads(1);
//   data_index->register_thread(0);

//   std::map<KeyT, std::pair<Uint64, ValueT>> validation_set;
//   std::vector<KeyT> keys_vector;

//   FastRandom rand;
//   // insert
//   for (size_t i = 0; i < n; ++i) {

//     KeyT key = rand.next<KeyT>();
//     ValueT value = i + 2048;
    
//     OffsetT offset = data_table->insert_tuple(key, value);
    
//     validation_set.insert(
//       std::pair<KeyT, std::pair<Uint64, ValueT>>(
//         key, std::pair<Uint64, ValueT>(offset.raw_data(), value)));
//     keys_vector.push_back(key);

//     data_index->insert(key, offset.raw_data());
//   }
//   // sort vector
//   std::sort(keys_vector.begin(), keys_vector.end());
  
//   // find
//   for (size_t i = 0; i < n / 2; i += 10) {
//     KeyT lower_key = keys_vector.at(i);
//     KeyT upper_key = keys_vector.at(keys_vector.size() - 1 - i);

//     std::vector<Uint64> offsets;
//     data_index->find_range(lower_key, upper_key, offsets);

//     auto lower_bound = validation_set.lower_bound(lower_key);
//     auto upper_bound = validation_set.upper_bound(upper_key);

//     std::unordered_set<Uint64> real_offsets;
//     for (auto iter = lower_bound; iter != upper_bound; ++iter) {

//       real_offsets.insert(iter->second.first);
//     }

//     EXPECT_EQ(real_offsets.size(), offsets.size());

//     for (auto entry : real_offsets) {

//       EXPECT_NE(real_offsets.end(), real_offsets.find(entry));
//     }
//   }
// }

// TEST_F(DynamicIndexNumericTest, UniqueKeyFindRangeTest) {

//   std::vector<IndexType> index_types {

//     // dynamic indexes - singlethread
//     IndexType::D_ST_StxBtree,
//     // IndexType::D_ST_ArtTree,
//     // IndexType::D_ST_Btree, // unimplemented
    
//     // dynamic indexes - multithread
//     // IndexType::D_MT_Libcuckoo, // do not support range queries
//     // IndexType::D_MT_ArtTree,
//     IndexType::D_MT_BwTree,
//     // IndexType::D_MT_Masstree,
//   };

//   for (auto index_type : index_types) {
//     // key type is set to uint16_t
//     // test_dynamic_index_numeric_unique_key_find_range<uint16_t, uint64_t>(index_type);
    
//     // key type is set to uint32_t
//     test_dynamic_index_numeric_unique_key_find_range<uint32_t, uint64_t>(index_type);
    
//     // key type is set to uint64_t
//     test_dynamic_index_numeric_unique_key_find_range<uint64_t, uint64_t>(index_type);
//   }

// }

// template<typename KeyT, typename ValueT>
// void test_dynamic_index_numeric_non_unique_key_find_range(const IndexType index_type) {

//   size_t n = 10000;
//   size_t m = 1000;
  
//   FastRandom rand_gen(0);

//   std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
//     new DataTable<KeyT, ValueT>());
//   std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
//     create_index<KeyT, ValueT>(index_type, data_table.get()));

//   data_index->prepare_threads(1);
//   data_index->register_thread(0);

//   std::map<KeyT, std::unordered_map<Uint64, ValueT>> validation_set;
//   std::vector<KeyT> keys_vector;
  
//   // insert
//   for (size_t i = 0; i < n; ++i) {

//     KeyT key = rand_gen.next<KeyT>() % m;
//     ValueT value = i + 2048;
    
//     OffsetT offset = data_table->insert_tuple(key, value);
    
//     validation_set[key][offset.raw_data()] = value;
//     keys_vector.push_back(key);

//     data_index->insert(key, offset.raw_data());
//   }

//   // sort vector
//   std::sort(keys_vector.begin(), keys_vector.end());

//   // find
//   for (size_t i = 0; i < n / 2; i += 10) {
//     KeyT lower_key = keys_vector.at(i);
//     KeyT upper_key = keys_vector.at(keys_vector.size() - 1 - i);

//     std::vector<Uint64> offsets;
//     data_index->find_range(lower_key, upper_key, offsets);

//     auto lower_bound = validation_set.lower_bound(lower_key);
//     auto upper_bound = validation_set.upper_bound(upper_key);

//     std::unordered_set<Uint64> real_offsets;
//     for (auto iter = lower_bound; iter != upper_bound; ++iter) {

//       for (auto entry : iter->second) {
//         real_offsets.insert(entry.first);
//       }
//     }
    
//     EXPECT_EQ(real_offsets.size(), offsets.size());

//     for (auto entry : real_offsets) {

//       EXPECT_NE(real_offsets.end(), real_offsets.find(entry));      
//     }

//   }
// }


// TEST_F(DynamicIndexNumericTest, NonUniqueKeyFindRangeTest) {

//   std::vector<IndexType> index_types {

//     // dynamic indexes - singlethread
//     IndexType::D_ST_StxBtree,
//     // IndexType::D_ST_ArtTree, // do not support non-unique keys
//     // IndexType::D_ST_Btree, // unimplemented
    
//     // dynamic indexes - multithread
//     // IndexType::D_MT_Libcuckoo, // do not support range queries
//     // IndexType::D_MT_ArtTree,
//     IndexType::D_MT_BwTree,
//     // IndexType::D_MT_Masstree, // do not support non-unique keys
//   };

//   for (auto index_type : index_types) {

//     // key type is set to uint16_t
//     // test_dynamic_index_numeric_non_unique_key_find_range<uint16_t, uint64_t>(index_type);

//     // key type is set to uint32_t
//     test_dynamic_index_numeric_non_unique_key_find_range<uint32_t, uint64_t>(index_type);

//     // key type is set to uint64_t
//     test_dynamic_index_numeric_non_unique_key_find_range<uint64_t, uint64_t>(index_type);
//   }
// }





