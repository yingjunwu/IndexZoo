#include <map>
#include <unordered_map>
#include <vector>

#include "harness.h"
#include "fast_random.h"
#include "time_measurer.h"

#include "data_table.h"

#include "index_all.h"


class StaticIndexTest : public IndexZooTest {};

template<typename KeyT, typename ValueT>
void test_unique_key(const IndexType index_type) {

  size_t n = 10000;

  std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
    new DataTable<KeyT, ValueT>());
  std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
    create_index<KeyT, ValueT>(index_type, data_table.get(), 5, 5));

  std::unordered_map<KeyT, std::pair<Uint64, ValueT>> validation_set;
  
  // insert
  for (size_t i = 0; i < n; ++i) {

    KeyT key = n - i - 1;
    ValueT value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    
    validation_set.insert(
      std::pair<KeyT, std::pair<Uint64, ValueT>>(
        key, std::pair<Uint64, ValueT>(offset.raw_data(), value)));

  }

  // reorganize data
  data_index->reorganize();

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

TEST_F(StaticIndexTest, UniqueKeyTest) {

  // std::vector<IndexType> index_types {

  //   IndexType::S_Interpolation,
  //   IndexType::S_Binary,
  //   IndexType::S_KAry,
  //   // IndexType::S_Fast,
  // };

  // for (auto index_type : index_types) {
  //   // key type is set to uint16_t
  //   test_unique_key<uint16_t, uint64_t>(index_type);
    
  //   // key type is set to uint32_t
  //   test_unique_key<uint32_t, uint64_t>(index_type);
    
  //   // key type is set to uint64_t
  //   test_unique_key<uint64_t, uint64_t>(index_type);
  // }

  IndexType index_type = IndexType::S_Fast;
  test_unique_key<uint32_t, uint64_t>(index_type);

}


template<typename KeyT, typename ValueT>
void test_non_unique_key(const IndexType index_type) {

  size_t n = 10000;
  size_t m = 1000;
  
  FastRandom rand_gen(0);

  std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
    new DataTable<KeyT, ValueT>());
  std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
    create_index<KeyT, ValueT>(index_type, data_table.get(), 1, 2));

  std::unordered_map<KeyT, std::unordered_map<Uint64, ValueT>> validation_set;

  // insert
  for (size_t i = 0; i < n; ++i) {

    KeyT key = rand_gen.next<KeyT>() / m;
    ValueT value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    
    validation_set[key][offset.raw_data()] = value;
  }

  // reorganize data
  data_index->reorganize();

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

// TEST_F(StaticIndexTest, NonUniqueKeyTest) {

//   std::vector<IndexType> index_types {

//     // IndexType::S_Interpolation,
//     // IndexType::S_Binary,
//     // IndexType::S_KAry,
//     // IndexType::S_Fast,
//   };

//   for (auto index_type : index_types) {

//     // key type is set to uint16_t
//     test_non_unique_key<uint16_t, uint64_t>(index_type);

//     // key type is set to uint32_t
//     test_non_unique_key<uint32_t, uint64_t>(index_type);

//     // key type is set to uint64_t
//     test_non_unique_key<uint64_t, uint64_t>(index_type);

//   }

// }