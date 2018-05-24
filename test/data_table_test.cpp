#include <map>
#include <unordered_map>
#include <vector>

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
  data_table_numeric_test<uint32_t>();
  data_table_numeric_test<uint64_t>();
}


template<size_t KeySize>
void data_table_generic_test() {
  // size_t n = 54321;
  size_t n = 1000;

  std::vector<std::pair<char*, uint64_t>> validation_vector;
  std::vector<std::pair<char*, uint64_t>> test_vector;

  std::unique_ptr<GenericDataTable<KeySize, uint64_t>> data_table(
    new GenericDataTable<KeySize, uint64_t>());

  FastRandom fast_rand(0);

  GenericKey<KeySize> key;
  // insert
  for (size_t i = 0; i < n; ++i) {

    fast_rand.next_chars(KeySize, key.raw());
    uint64_t value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key.raw(), value);
    
    validation_vector.emplace_back(std::pair<char*, uint64_t>(key.raw(), offset.raw_data()));
  }

  GenericDataTableIterator<KeySize, uint64_t> iterator(data_table.get());
  while (iterator.has_next()) {
    auto entry = iterator.next();
    test_vector.emplace_back(std::pair<char*, uint64_t>(entry.key_, entry.offset_));
  }

  EXPECT_EQ(validation_vector.size(), n);
  EXPECT_EQ(test_vector.size(), n);

  for (size_t i = 0; i < test_vector.size(); ++i) {
    EXPECT_EQ(strcmp(test_vector.at(i).first, validation_vector.at(i).first), 0);
    EXPECT_EQ(test_vector.at(i).second, validation_vector.at(i).second);
  }
}



TEST_F(DataTableTest, GenericTest) {
  data_table_generic_test<8>();
}
