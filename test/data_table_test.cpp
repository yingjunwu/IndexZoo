#include <map>
#include <unordered_map>
#include <vector>

#include "generic_key.h"
#include "generic_data_table.h"
#include "data_table.h"
#include "fast_random.h"

#include "harness.h"


class DataTableTest : public IndexZooTest {};


template<typename KeyT>
void data_table_numeric_test() {
  // size_t n = 54321;
  size_t n = 1000;

  std::vector<std::pair<KeyT, uint64_t>> validation_vector;
  std::vector<std::pair<KeyT, uint64_t>> test_vector;

  std::unique_ptr<DataTable<KeyT, uint64_t>> data_table(
    new DataTable<KeyT, uint64_t>());

  // insert
  for (size_t i = 0; i < n; ++i) {

    KeyT key = n - i - 1;
    uint64_t value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    
    validation_vector.emplace_back(std::pair<KeyT, uint64_t>(key, offset.raw_data()));
  }

  DataTableIterator<KeyT, uint64_t> iterator(data_table.get());
  while (iterator.has_next()) {
    auto entry = iterator.next();
    test_vector.emplace_back(std::pair<KeyT, uint64_t>(*(entry.key_), entry.offset_));
  }

  EXPECT_EQ(validation_vector.size(), n);
  EXPECT_EQ(test_vector.size(), n);

  for (size_t i = 0; i < test_vector.size(); ++i) {
    EXPECT_EQ(test_vector.at(i).first, validation_vector.at(i).first);
    EXPECT_EQ(test_vector.at(i).second, validation_vector.at(i).second);
  }
}

TEST_F(DataTableTest, NumericTest) {
  data_table_numeric_test<uint16_t>();
  data_table_numeric_test<uint32_t>();
  data_table_numeric_test<uint64_t>();
}


void data_table_generic_test(const uint64_t max_key_size) {
  // size_t n = 54321;
  size_t n = 1000;

  std::vector<std::pair<char*, uint64_t>> validation_vector;
  std::vector<std::pair<char*, uint64_t>> test_vector;

  std::unique_ptr<GenericDataTable<uint64_t>> data_table(
    new GenericDataTable<uint64_t>(max_key_size));

  FastRandom fast_rand(0);

  uint64_t key_size = max_key_size - 1;

  GenericKey key(key_size);
  // insert
  for (size_t i = 0; i < n; ++i) {

    fast_rand.next_readable_chars(key_size, key.raw());
    uint64_t value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key.raw(), key_size, value);

    char *tmp = new char[key_size];
    memcpy(tmp, key.raw(), key_size);
    validation_vector.emplace_back(std::pair<char*, uint64_t>(tmp, offset.raw_data()));
  }

  GenericDataTableIterator<uint64_t> iterator(data_table.get());
  while (iterator.has_next()) {
    auto entry = iterator.next();
    test_vector.emplace_back(std::pair<char*, uint64_t>(entry.key_, entry.offset_));
  }

  EXPECT_EQ(validation_vector.size(), n);
  EXPECT_EQ(test_vector.size(), n);

  for (size_t i = 0; i < test_vector.size(); ++i) {
    EXPECT_EQ(strncmp(test_vector.at(i).first, validation_vector.at(i).first, key_size), 0);
    EXPECT_EQ(test_vector.at(i).second, validation_vector.at(i).second);
  }

  for (size_t i = 0; i < test_vector.size(); ++i) {
    delete[] validation_vector.at(i).first;
  }
}

TEST_F(DataTableTest, GenericTest) {
  data_table_generic_test(16);
}
