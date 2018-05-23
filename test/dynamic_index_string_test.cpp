#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "harness.h"
#include "fast_random.h"
#include "time_measurer.h"

#include "data_table.h"

#include "index_all.h"


class DynamicIndexStringTest : public IndexZooTest {};

template<size_t KeySize, typename ValueT>
void test_dynamic_index_string_unique_key(const IndexType index_type, const size_t index_param_1, const size_t index_param_2) {

  size_t n = 10000;

  // std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
  //   new DataTable<KeyT, ValueT>());
  // std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
  //   create_index<KeyT, ValueT>(index_type, data_table.get(), index_param_1, index_param_2));

  // std::unordered_map<StringKey<KeySize>, std::pair<Uint64, ValueT>> validation_set;
  
  std::map<StringKey<KeySize>, uint64_t, StringComparator<KeySize>> my_map;
  std::map<std::string, uint64_t> my_map1;


  FastRandom fast_rand(0);
  // insert
  for (size_t i = 0; i < 10; ++i) {
    
    StringKey<KeySize> s;
    fast_rand.next_chars(KeySize, s.get_chars());
    
    // // fast_rand.next_chars(KeySize - 1, s.data_);
    
    // char data[KeySize];
    // fast_rand.next_chars(KeySize, data);



    // fast_rand.next_chars(KeySize - 1, s.get_chars());
    // my_map[s] = 100;

    // std::string s;
    // fast_rand.next_string(KeySize - 1, s);

    my_map[s] = 100;
  }

  for (auto entry : my_map) {
    std::cout << my_map.first << " " << my_map.second << std::endl;
  }

  // stx::btree_multimap<StringKey<KeySize>, Uint64> container;

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


TEST_F(DynamicIndexStringTest, RandomKeyTest) {
  
  test_dynamic_index_string_unique_key<32, uint64_t>(IndexType::D_ST_ArtTree, INVALID_INDEX_PARAM, INVALID_INDEX_PARAM);

  // for (auto index_type : index_types) {
  //   // key type is set to uint16_t
  //   test_dynamic_index_numeric_unique_key<uint16_t, uint64_t>(index_type);
    
  //   // key type is set to uint32_t
  //   test_dynamic_index_numeric_unique_key<uint32_t, uint64_t>(index_type);
    
  //   // key type is set to uint64_t
  //   test_dynamic_index_numeric_unique_key<uint64_t, uint64_t>(index_type);
  // }


}
