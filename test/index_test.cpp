#include <map>
#include <unordered_map>
#include <vector>

#include "harness.h"
#include "fast_random.h"
#include "time_measurer.h"

#include "data_table.h"
#include "learned_index.h"


class IndexTest : public LearnedIndexTest {};

typedef uint64_t ValueT;

// TEST_F(IndexTest, LearnedIndex) {

//   std::unique_ptr<LearnedIndex<uint64_t>> data_index(new LearnedIndex<uint64_t>());

//   FastRandom rand_gen;

//   size_t n = 10;
//   for (size_t i = 0; i < n; ++i) {
//     data_index->insert(rand_gen.next() % n, rand_gen.next() % n);
//   }
//   std::cout << data_index->size() << std::endl;
//   data_index->reorganize();
//   data_index->print();
// }


TEST_F(IndexTest, UniqueKeyTest) {

  size_t n = 100000;

  std::unique_ptr<DataTable<uint64_t, uint64_t>> data_table(
    new DataTable<uint64_t, uint64_t>());
  std::unique_ptr<LearnedIndex<uint64_t>> data_index(
    new LearnedIndex<uint64_t>());

  std::unordered_map<uint64_t, std::pair<Uint64, uint64_t>> validation_set;
  
  // insert
  for (size_t i = 0; i < n; ++i) {

    uint64_t key = i;
    uint64_t value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    
    validation_set.insert(
      std::pair<uint64_t, std::pair<Uint64, uint64_t>>(
        key, std::pair<Uint64, uint64_t>(offset.raw_data(), value)));

    data_index->insert(key, offset.raw_data());
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

