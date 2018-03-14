#include <map>
#include <unordered_map>
#include <vector>

#include "data_table.h"

#include "harness.h"


class DataTableTest : public IndexZooTest {};


TEST_F(DataTableTest, IteratorTest) {

  // size_t n = 54321;
  size_t n = 1000;

  std::vector<std::pair<Uint64, uint64_t>> validation_vector;
  std::vector<std::pair<Uint64, uint64_t>> test_vector;

  std::unique_ptr<DataTable<uint64_t, uint64_t>> data_table(
    new DataTable<uint64_t, uint64_t>());

  // insert
  for (size_t i = 0; i < n; ++i) {

    uint64_t key = n - i - 1;
    uint64_t value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    
    validation_vector.emplace_back(std::pair<Uint64, uint64_t>(offset.raw_data(), key));
  }

  DataTableIterator<uint64_t, uint64_t> iterator(data_table.get());
  while (iterator.has_next()) {
    auto entry = iterator.next();
    test_vector.emplace_back(std::pair<Uint64, uint64_t>(entry.offset_, *(entry.key_)));
  }

  EXPECT_EQ(validation_vector.size(), n);
  EXPECT_EQ(test_vector.size(), n);

  for (size_t i = 0; i < test_vector.size(); ++i) {
    EXPECT_EQ(test_vector.at(i).first, validation_vector.at(i).first);
    EXPECT_EQ(test_vector.at(i).second, validation_vector.at(i).second);
  }

}

