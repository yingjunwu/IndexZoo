#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "harness.h"
#include "fast_random.h"
#include "time_measurer.h"

#include "data_table.h"

#include "index_all.h"


class StaticIndexNumericTest : public IndexZooTest {};

template<typename KeyT, typename ValueT>
void test_static_index_numeric_unique_key_find(const IndexType index_type, const size_t index_param_1, const size_t index_param_2) {

  size_t n = 10000;

  std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
    new DataTable<KeyT, ValueT>());
  std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
    create_index<KeyT, ValueT>(index_type, data_table.get(), index_param_1, index_param_2));

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

TEST_F(StaticIndexNumericTest, UniqueKeyFindTest) {
  
  IndexType index_type = IndexType::S_Interpolation;
  for (size_t segments = 1; segments <= 10; ++segments) {
    test_static_index_numeric_unique_key_find<uint16_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
    test_static_index_numeric_unique_key_find<uint32_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
    test_static_index_numeric_unique_key_find<uint64_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
  }

  index_type = IndexType::S_Binary;
  for (size_t layers = 0; layers < 8; ++layers) {
    test_static_index_numeric_unique_key_find<uint16_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
    test_static_index_numeric_unique_key_find<uint32_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
    test_static_index_numeric_unique_key_find<uint64_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  }

  index_type = IndexType::S_KAry;
  for (size_t layers = 0; layers < 4; ++layers) {
    for (size_t k = 2; k < 5; ++k) {
      test_static_index_numeric_unique_key_find<uint16_t, uint64_t>(index_type, layers, k);
      test_static_index_numeric_unique_key_find<uint32_t, uint64_t>(index_type, layers, k);
      test_static_index_numeric_unique_key_find<uint64_t, uint64_t>(index_type, layers, k);
    }
  }

  index_type = IndexType::S_Fast;
  for (size_t layers = 0; layers <= 12; layers += 4) {
    test_static_index_numeric_unique_key_find<uint32_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  }

}


template<typename KeyT, typename ValueT>
void test_static_index_numeric_non_unique_key_find(const IndexType index_type, const size_t index_param_1, const size_t index_param_2) {

  size_t n = 10000;
  size_t m = 1000;
  
  FastRandom rand_gen(0);

  std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
    new DataTable<KeyT, ValueT>());
  std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
    create_index<KeyT, ValueT>(index_type, data_table.get(), index_param_1, index_param_2));

  std::unordered_map<KeyT, std::unordered_map<Uint64, ValueT>> validation_set;

  // insert
  for (size_t i = 0; i < n; ++i) {

    KeyT key = rand_gen.next<KeyT>() % m;
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

TEST_F(StaticIndexNumericTest, NonUniqueKeyFindTest) {

  IndexType index_type = IndexType::S_Interpolation;
  for (size_t segments = 1; segments <= 10; ++segments) {
    test_static_index_numeric_non_unique_key_find<uint16_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
    test_static_index_numeric_non_unique_key_find<uint32_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
    test_static_index_numeric_non_unique_key_find<uint64_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
  }

  index_type = IndexType::S_Binary;
  for (size_t layers = 0; layers < 8; ++layers) {
    test_static_index_numeric_non_unique_key_find<uint16_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
    test_static_index_numeric_non_unique_key_find<uint32_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
    test_static_index_numeric_non_unique_key_find<uint64_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  }

  index_type = IndexType::S_KAry;
  for (size_t layers = 0; layers < 4; ++layers) {
    for (size_t k = 2; k < 5; ++k) {
      test_static_index_numeric_non_unique_key_find<uint16_t, uint64_t>(index_type, layers, k);
      test_static_index_numeric_non_unique_key_find<uint32_t, uint64_t>(index_type, layers, k);
      test_static_index_numeric_non_unique_key_find<uint64_t, uint64_t>(index_type, layers, k);
    }
  }

  index_type = IndexType::S_Fast;
  for (size_t layers = 0; layers <= 12; layers += 4) {
    test_static_index_numeric_non_unique_key_find<uint32_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  }

}


template<typename KeyT, typename ValueT>
void test_static_index_numeric_unique_key_find_range(const IndexType index_type, const size_t index_param_1, const size_t index_param_2) {

  size_t n = 10000;

  std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
    new DataTable<KeyT, ValueT>());
  std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
    create_index<KeyT, ValueT>(index_type, data_table.get(), index_param_1, index_param_2));

  std::map<KeyT, std::pair<Uint64, ValueT>> validation_set;
  std::vector<KeyT> keys_vector;

  FastRandom rand;
  // insert
  for (size_t i = 0; i < n; ++i) {

    KeyT key = rand.next<KeyT>();
    ValueT value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    
    validation_set.insert(
      std::pair<KeyT, std::pair<Uint64, ValueT>>(
        key, std::pair<Uint64, ValueT>(offset.raw_data(), value)));
    keys_vector.push_back(key);
  }

  // reorganize data
  data_index->reorganize();

  // sort vector
  std::sort(keys_vector.begin(), keys_vector.end());
  
  // find
  for (size_t i = 0; i < n / 2; ++i) {
    KeyT lower_key = keys_vector.at(i);
    KeyT upper_key = keys_vector.at(keys_vector.size() - 1 - i);

    std::vector<Uint64> offsets;
    data_index->find_range(lower_key, upper_key, offsets);

    auto lower_bound = validation_set.lower_bound(lower_key);
    auto upper_bound = validation_set.upper_bound(upper_key);

    std::unordered_set<Uint64> real_offsets;
    for (auto iter = lower_bound; iter != upper_bound; ++iter) {

      real_offsets.insert(iter->second.first);
    }

    EXPECT_EQ(real_offsets.size(), offsets.size());

    for (auto entry : real_offsets) {

      EXPECT_NE(real_offsets.end(), real_offsets.find(entry));
    }
  }
}

TEST_F(StaticIndexNumericTest, UniqueKeyFindRangeTest) {

  IndexType index_type = IndexType::S_Interpolation;
  for (size_t segments = 1; segments <= 10; ++segments) {
    // test_static_index_numeric_unique_key_find_range<uint16_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
    // test_static_index_numeric_unique_key_find_range<uint32_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
    test_static_index_numeric_unique_key_find_range<uint64_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
  }

  // index_type = IndexType::S_Binary;
  // for (size_t layers = 0; layers < 8; ++layers) {
  //   test_static_index_numeric_unique_key_find_range<uint16_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  //   test_static_index_numeric_unique_key_find_range<uint32_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  //   test_static_index_numeric_unique_key_find_range<uint64_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  // }

  // index_type = IndexType::S_KAry;
  // for (size_t layers = 0; layers < 4; ++layers) {
  //   for (size_t k = 2; k < 5; ++k) {
  //     test_static_index_numeric_unique_key_find_range<uint16_t, uint64_t>(index_type, layers, k);
  //     test_static_index_numeric_unique_key_find_range<uint32_t, uint64_t>(index_type, layers, k);
  //     test_static_index_numeric_unique_key_find_range<uint64_t, uint64_t>(index_type, layers, k);
  //   }
  // }

  // index_type = IndexType::S_Fast;
  // for (size_t layers = 0; layers <= 12; layers += 4) {
  //   test_static_index_numeric_unique_key_find_range<uint32_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  // }

}

template<typename KeyT, typename ValueT>
void test_static_index_numeric_non_unique_key_find_range(const IndexType index_type, const size_t index_param_1, const size_t index_param_2) {

  size_t n = 10000;
  size_t m = 1000;
  
  FastRandom rand_gen(0);

  std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
    new DataTable<KeyT, ValueT>());
  std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
    create_index<KeyT, ValueT>(index_type, data_table.get(), index_param_1, index_param_2));

  std::map<KeyT, std::unordered_map<Uint64, ValueT>> validation_set;
  std::vector<KeyT> keys_vector;
  
  // insert
  for (size_t i = 0; i < n; ++i) {

    KeyT key = rand_gen.next<KeyT>() % m;
    ValueT value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    
    validation_set[key][offset.raw_data()] = value;
    keys_vector.push_back(key);
  }

  // reorganize data
  data_index->reorganize();

  // sort vector
  std::sort(keys_vector.begin(), keys_vector.end());

  // find
  for (size_t i = 0; i < n / 2; ++i) {
    KeyT lower_key = keys_vector.at(i);
    KeyT upper_key = keys_vector.at(keys_vector.size() - 1 - i);

    std::vector<Uint64> offsets;
    data_index->find_range(lower_key, upper_key, offsets);

    auto lower_bound = validation_set.lower_bound(lower_key);
    auto upper_bound = validation_set.upper_bound(upper_key);

    std::unordered_set<Uint64> real_offsets;
    for (auto iter = lower_bound; iter != upper_bound; ++iter) {

      for (auto entry : iter->second) {
        real_offsets.insert(entry.first);
      }
    }
    
    EXPECT_EQ(real_offsets.size(), offsets.size());

    for (auto entry : real_offsets) {

      EXPECT_NE(real_offsets.end(), real_offsets.find(entry));      
    }

  }
}


TEST_F(StaticIndexNumericTest, NonUniqueKeyFindRangeTest) {

  // IndexType index_type = IndexType::S_Interpolation;
  // for (size_t segments = 1; segments <= 10; ++segments) {
  //   test_static_index_numeric_non_unique_key_find_range<uint16_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
  //   test_static_index_numeric_non_unique_key_find_range<uint32_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
  //   test_static_index_numeric_non_unique_key_find_range<uint64_t, uint64_t>(index_type, segments, INVALID_INDEX_PARAM);
  // }

  // index_type = IndexType::S_Binary;
  // for (size_t layers = 0; layers < 8; ++layers) {
  //   test_static_index_numeric_non_unique_key_find_range<uint16_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  //   test_static_index_numeric_non_unique_key_find_range<uint32_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  //   test_static_index_numeric_non_unique_key_find_range<uint64_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  // }

  // index_type = IndexType::S_KAry;
  // for (size_t layers = 0; layers < 4; ++layers) {
  //   for (size_t k = 2; k < 5; ++k) {
  //     test_static_index_numeric_non_unique_key_find_range<uint16_t, uint64_t>(index_type, layers, k);
  //     test_static_index_numeric_non_unique_key_find_range<uint32_t, uint64_t>(index_type, layers, k);
  //     test_static_index_numeric_non_unique_key_find_range<uint64_t, uint64_t>(index_type, layers, k);
  //   }
  // }

  // index_type = IndexType::S_Fast;
  // for (size_t layers = 0; layers <= 12; layers += 4) {
  //   test_static_index_numeric_non_unique_key_find_range<uint32_t, uint64_t>(index_type, layers, INVALID_INDEX_PARAM);
  // }
}














