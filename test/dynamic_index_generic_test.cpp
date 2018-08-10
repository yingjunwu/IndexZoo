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

template<size_t MaxKeySize, typename ValueT>
void test_dynamic_index_generic_unique_key(const IndexType index_type) {

  size_t n = 10000;

  std::unique_ptr<GenericDataTable<ValueT>> data_table(
    new GenericDataTable<ValueT>(MaxKeySize));

  // data_index->prepare_threads(1);
  // data_index->register_thread(0);

  std::unordered_map<std::string, std::pair<Uint64, ValueT>> validation_set;
  
  FastRandom fast_rand(0);

  size_t key_size = MaxKeySize - 1;
  GenericKey key(key_size);
  // insert
  // for (size_t i = 0; i < 10; ++i) {
    
  //   fast_rand.next_chars(MaxKeySize, key.raw());
  //   ValueT value = i + 2048;

  //   OffsetT offset = data_table->insert_tuple(key, value);
    
  // }

  // stx::btree_multimap<GenericKey<MaxKeySize>, Uint64> container;

  // // insert
  // for (size_t i = 0; i < n; ++i) {

  //   KeyT key = n - i - 1;
  //   ValueT value = i + 2048;
    
  //   OffsetT offset = data_table->insert_tuple(key, value);
    
  //   validation_set.insert(
  //     std::pair<KeyT, std::pair<Uint64, ValueT>>(
  //       key, std::pair<Uint64, ValueT>(offset.raw_data(), value)));

  // }

  // // reorganize data
  // data_index->reorganize();

  // // find
  // for (size_t i = 0; i < n; ++i) {
  //   KeyT key = i;

  //   std::vector<Uint64> offsets;

  //   data_index->find(key, offsets);

  //   EXPECT_EQ(offsets.size(), 1);

  //   ValueT *value = data_table->get_tuple_value(offsets.at(0));

  //   EXPECT_EQ(offsets.at(0), validation_set.find(key)->second.first);

  //   EXPECT_EQ(*value, validation_set.find(key)->second.second);
  // }
}


TEST_F(DynamicIndexGenericTest, RandomKeyTest) {
  
  test_dynamic_index_generic_unique_key<32, uint64_t>(IndexType::D_ST_ArtTree);

  // for (auto index_type : index_types) {
  //   // key type is set to uint16_t
  //   test_dynamic_index_numeric_unique_key<uint16_t, uint64_t>(index_type);
    
  //   // key type is set to uint32_t
  //   test_dynamic_index_numeric_unique_key<uint32_t, uint64_t>(index_type);
    
  //   // key type is set to uint64_t
  //   test_dynamic_index_numeric_unique_key<uint64_t, uint64_t>(index_type);
  // }


}
