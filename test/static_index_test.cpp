#include <map>
#include <unordered_map>
#include <vector>

#include "harness.h"
#include "fast_random.h"
#include "time_measurer.h"

#include "data_table.h"

#include "index_all.h"


class StaticIndexTest : public IndexZooTest {};

typedef uint64_t ValueT;

TEST_F(StaticIndexTest, UniqueKeyTest) {

  std::vector<StaticIndexType> index_types {

    StaticIndexType::InterpolationIndexType,
    // StaticIndexType::KAryIndexType,
  };

  size_t n = 1000;

  for (auto index_type : index_types) {

  std::unique_ptr<DataTable<uint64_t, uint64_t>> data_table(
    new DataTable<uint64_t, uint64_t>());
  std::unique_ptr<BaseStaticIndex<uint64_t>> data_index(
    create_static_index<uint64_t>(index_type, data_table.get(), 1));

  std::unordered_map<uint64_t, std::pair<Uint64, uint64_t>> validation_set;
  
  // insert
  for (size_t i = 0; i < n; ++i) {

    uint64_t key = n - i - 1;
    uint64_t value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    
    validation_set.insert(
      std::pair<uint64_t, std::pair<Uint64, uint64_t>>(
        key, std::pair<Uint64, uint64_t>(offset.raw_data(), value)));

  }

  // reorganize data
  data_index->reorganize();

  // find
  for (size_t i = 0; i < n; ++i) {
    uint64_t key = i;

    std::vector<Uint64> offsets;

    data_index->find(key, offsets);

    EXPECT_EQ(offsets.size(), 1);

    uint64_t *value = data_table->get_tuple_value(offsets.at(0));

    EXPECT_EQ(offsets.at(0), validation_set.find(key)->second.first);

    EXPECT_EQ(*value, validation_set.find(key)->second.second);
  }

  }

}


TEST_F(StaticIndexTest, NonUniqueKeyTest) {

  std::vector<StaticIndexType> index_types {

    StaticIndexType::InterpolationIndexType,
    // StaticIndexType::KAryIndexType,
  };

  size_t n = 10000;
  size_t m = 1000;

  for (auto index_type : index_types) {

    FastRandom rand_gen(0);

    std::unique_ptr<DataTable<uint64_t, uint64_t>> data_table(
      new DataTable<uint64_t, uint64_t>());
    std::unique_ptr<BaseStaticIndex<uint64_t>> data_index(
      create_static_index<uint64_t>(index_type, data_table.get(), 1));

    std::unordered_map<uint64_t, std::unordered_map<Uint64, uint64_t>> validation_set;
    
    // insert
    for (size_t i = 0; i < n; ++i) {

      uint64_t key = rand_gen.next() / m;
      uint64_t value = i + 2048;
      
      OffsetT offset = data_table->insert_tuple(key, value);
      
      validation_set[key][offset.raw_data()] = value;
    }

    // reorganize data
    data_index->reorganize();

    // find
    for (auto entry : validation_set) {
      uint64_t key = entry.first;

      std::vector<Uint64> offsets;

      data_index->find(key, offsets);

      EXPECT_EQ(offsets.size(), entry.second.size());

      for (auto offset : offsets) {
        uint64_t *value = data_table->get_tuple_value(offset);

        EXPECT_NE(entry.second.end(), entry.second.find(offset));

        EXPECT_EQ(*value, entry.second.at(offset));
      }
    }
  }

}