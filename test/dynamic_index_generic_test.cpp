#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "harness.h"
#include "fast_random.h"
#include "time_measurer.h"

#include "generic_key.h"
#include "generic_data_table.h"

#include "index_all.h"


class DynamicIndexGenericTest : public IndexZooTest {};

void test_dynamic_index_generic_unique_key_find(const uint64_t max_key_size, const IndexType index_type) {

  size_t n = 10000;

  std::unique_ptr<GenericDataTable> data_table(
    new GenericDataTable(max_key_size, sizeof(uint64_t)));
  std::unique_ptr<BaseGenericIndex> data_index(
    create_generic_index(index_type, data_table.get()));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  std::map<GenericKey, std::pair<Uint64, uint64_t>> validation_set;
  
  FastRandom rand;

  size_t key_size = max_key_size;

  GenericKey key(key_size);

  // insert
  for (size_t i = 0; i < n; ++i) {

    rand.next_readable_chars(key_size, key.raw());

    ValueT value = i + 2048;

    OffsetT offset = data_table->insert_tuple(key.raw(), key.size(), (char*)(&value), sizeof(value));

    validation_set.insert(
      std::pair<GenericKey, std::pair<Uint64, uint64_t>>(
        key, std::pair<Uint64, uint64_t>(offset.raw_data(), value)));

    data_index->insert(key, offset.raw_data());
  }

  // find
  for (auto entry : validation_set) {
    GenericKey key = entry.first;

    std::vector<Uint64> offsets;

    data_index->find(key, offsets);

    EXPECT_EQ(offsets.size(), 1);

    char *value = data_table->get_tuple_value(offsets.at(0));

    EXPECT_EQ(offsets.at(0), entry.second.first);

    EXPECT_EQ((*(uint64_t*)value), entry.second.second);
  }
}


TEST_F(DynamicIndexGenericTest, UniqueKeyFindTest) {
  
  std::vector<IndexType> index_types {

    // dynamic indexes - singlethread
    IndexType::D_ST_StxBtree,
    IndexType::D_ST_ArtTree,
    
    // // dynamic indexes - multithread
    IndexType::D_MT_Libcuckoo,
    IndexType::D_MT_ArtTree,
    IndexType::D_MT_BwTree,
    IndexType::D_MT_Masstree,
  };

  for (auto index_type : index_types) {
    test_dynamic_index_generic_unique_key_find(32, index_type);

    test_dynamic_index_generic_unique_key_find(64, index_type);
  }

}


void test_dynamic_index_generic_non_unique_key_find(const uint64_t max_key_size, const IndexType index_type) {

  size_t n = 10000;
  size_t m = 1000;
  
  FastRandom rand_gen(0);

  std::unique_ptr<GenericDataTable> data_table(
    new GenericDataTable(max_key_size, sizeof(uint64_t)));
  std::unique_ptr<BaseGenericIndex> data_index(
    create_generic_index(index_type, data_table.get()));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  std::map<GenericKey, std::unordered_map<Uint64, uint64_t>> validation_set;
  
  std::vector<GenericKey> unique_keys;

  size_t key_size = max_key_size;
  
  GenericKey key(key_size);
  
  for (size_t i = 0; i < m; ++i) {
    rand_gen.next_readable_chars(key_size, key.raw());
    unique_keys.push_back(key);
  }

  // insert
  for (size_t i = 0; i < n; ++i) {

    uint64_t key_id = rand_gen.next<uint64_t>() % m;
    GenericKey key = unique_keys.at(key_id);

    ValueT value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key.raw(), key.size(), (char*)(&value), sizeof(uint64_t));
    
    validation_set[key][offset.raw_data()] = value;

    data_index->insert(key, offset.raw_data());
  }

  // find
  for (auto entry : validation_set) {
    GenericKey key = entry.first;

    std::vector<Uint64> offsets;

    data_index->find(key, offsets);

    EXPECT_EQ(offsets.size(), entry.second.size());

    for (auto offset : offsets) {
      char *value = data_table->get_tuple_value(offset);

      EXPECT_NE(entry.second.end(), entry.second.find(offset));

      EXPECT_EQ((*(uint64_t*)value), entry.second.at(offset));
    }
  }
}


TEST_F(DynamicIndexGenericTest, NonUniqueKeyFindTest) {

  std::vector<IndexType> index_types {

    // dynamic indexes - singlethread
    IndexType::D_ST_StxBtree,
    IndexType::D_ST_ArtTree,
    
    // dynamic indexes - multithread
    IndexType::D_MT_Libcuckoo,
    IndexType::D_MT_ArtTree,
    IndexType::D_MT_BwTree,
    // IndexType::D_MT_Masstree, // do not support non-unique keys
  };

  for (auto index_type : index_types) {
    test_dynamic_index_generic_non_unique_key_find(32, index_type);

    test_dynamic_index_generic_non_unique_key_find(64, index_type);
  }
}


void test_dynamic_index_generic_unique_key_find_range(const uint64_t max_key_size, const IndexType index_type) {

  size_t n = 10000;

  std::unique_ptr<GenericDataTable> data_table(
    new GenericDataTable(max_key_size, sizeof(uint64_t)));
  std::unique_ptr<BaseGenericIndex> data_index(
    create_generic_index(index_type, data_table.get()));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  std::map<GenericKey, std::pair<Uint64, uint64_t>> validation_set;
  std::vector<GenericKey> keys_vector;

  size_t key_size = max_key_size;
  GenericKey key(key_size);

  FastRandom rand;
  // insert
  for (size_t i = 0; i < n; ++i) {

    rand.next_readable_chars(key_size, key.raw());

    while (validation_set.find(key) != validation_set.end()) {
      rand.next_readable_chars(key_size, key.raw());
    }

    uint64_t value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key.raw(), key.size(), (char*)(&value), sizeof(uint64_t));
    
    validation_set.insert(
      std::pair<GenericKey, std::pair<Uint64, uint64_t>>(
        key, std::pair<Uint64, uint64_t>(offset.raw_data(), value)));
    keys_vector.push_back(key);

    data_index->insert(key, offset.raw_data());
  }
  // sort vector
  std::sort(keys_vector.begin(), keys_vector.end());
  
  // find
  for (size_t i = 0; i < n / 2; i += 100) {
    GenericKey lower_key = keys_vector.at(i);
    GenericKey upper_key = keys_vector.at(keys_vector.size() - 1 - i);

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

TEST_F(DynamicIndexGenericTest, UniqueKeyFindRangeTest) {

  std::vector<IndexType> index_types {

    // dynamic indexes - singlethread
    IndexType::D_ST_StxBtree,
    // IndexType::D_ST_ArtTree, // do not fully support range queries
    
    // dynamic indexes - multithread
    // IndexType::D_MT_Libcuckoo, // do not support range queries
    // IndexType::D_MT_ArtTree, // do not fully support range queries
    IndexType::D_MT_BwTree,
    // IndexType::D_MT_Masstree, // do not support range queries
  };

  for (auto index_type : index_types) {
    test_dynamic_index_generic_unique_key_find_range(32, index_type);
    
    test_dynamic_index_generic_unique_key_find_range(64, index_type);
  }

}


void test_dynamic_index_generic_non_unique_key_find_range(const uint64_t max_key_size, const IndexType index_type) {

  size_t n = 10000;
  size_t m = 1000;
  
  FastRandom rand_gen(0);

  std::unique_ptr<GenericDataTable> data_table(
    new GenericDataTable(max_key_size, sizeof(uint64_t)));
  std::unique_ptr<BaseGenericIndex> data_index(
    create_generic_index(index_type, data_table.get()));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  std::map<GenericKey, std::unordered_map<Uint64, uint64_t>> validation_set;
  std::vector<GenericKey> unique_keys;
  std::vector<GenericKey> keys_vector;

  size_t key_size = max_key_size;
  GenericKey rand_key(key_size);

  for (size_t i = 0; i < m; ++i) {
    rand_gen.next_readable_chars(key_size, rand_key.raw());
    unique_keys.push_back(rand_key);
  }
  
  // insert
  for (size_t i = 0; i < n; ++i) {

    uint64_t key_id = rand_gen.next<uint64_t>() % m;
    GenericKey key = unique_keys.at(key_id);

    uint64_t value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key.raw(), key.size(), (char*)(&value), sizeof(uint64_t));
    
    validation_set[key][offset.raw_data()] = value;
    keys_vector.push_back(key);

    data_index->insert(key, offset.raw_data());
  }

  // sort vector
  std::sort(keys_vector.begin(), keys_vector.end());

  // find
  for (size_t i = 0; i < n / 2; i += 100) {
    GenericKey lower_key = keys_vector.at(i);
    GenericKey upper_key = keys_vector.at(keys_vector.size() - 1 - i);

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


TEST_F(DynamicIndexGenericTest, NonUniqueKeyFindRangeTest) {

  std::vector<IndexType> index_types {

    // dynamic indexes - singlethread
    IndexType::D_ST_StxBtree,
    // IndexType::D_ST_ArtTree, // do not support non-unique keys
    
    // dynamic indexes - multithread
    // IndexType::D_MT_Libcuckoo, // do not support range queries
    // IndexType::D_MT_ArtTree, // do not fully support range queries
    IndexType::D_MT_BwTree,
    // IndexType::D_MT_Masstree, // do not support non-unique keys
  };

  for (auto index_type : index_types) {

    test_dynamic_index_generic_non_unique_key_find_range(32, index_type);

    test_dynamic_index_generic_non_unique_key_find_range(64, index_type);

  }
}


void test_dynamic_index_generic_scan(const uint64_t max_key_size, const IndexType index_type) {

  size_t n = 10000;

  std::unique_ptr<GenericDataTable> data_table(
    new GenericDataTable(max_key_size, sizeof(uint64_t)));
  std::unique_ptr<BaseGenericIndex> data_index(
    create_generic_index(index_type, data_table.get()));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  std::map<GenericKey, std::pair<Uint64, uint64_t>> validation_set;

  FastRandom rand;

  GenericKey key(max_key_size);
  // insert
  for (size_t i = 0; i < n; ++i) {

    rand.next_readable_chars(max_key_size, key.raw());
    uint64_t value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key.raw(), key.size(), (char*)(&value), sizeof(uint64_t));
    
    validation_set.insert(
      std::pair<GenericKey, std::pair<Uint64, uint64_t>>(
        key, std::pair<Uint64, uint64_t>(offset.raw_data(), value)));

    data_index->insert(key, offset.raw_data());
  }
  
  // find
  for (size_t i = 1; i < n; i += 1000) {

    std::vector<Uint64> offsets;
    data_index->scan_full(offsets, i);

    EXPECT_EQ(offsets.size(), i);

    int count = 0;
    for (auto iter = validation_set.begin(); iter != validation_set.end(); ++iter) {
      if (count >= offsets.size()) {
        break;
      }
      EXPECT_EQ(offsets.at(count), iter->second.first);
      ++count;
    }
  }
}


TEST_F(DynamicIndexGenericTest, ScanTest) {

  std::vector<IndexType> index_types {
    IndexType::D_ST_StxBtree,
    IndexType::D_ST_ArtTree,
  };

  for (auto index_type : index_types) {
    test_dynamic_index_generic_scan(32, index_type);
    test_dynamic_index_generic_scan(64, index_type);

  }
}


