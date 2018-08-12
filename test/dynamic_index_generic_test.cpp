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

template<typename ValueT>
void test_dynamic_index_generic_unique_key_find(const uint64_t max_key_size, const IndexType index_type) {

  size_t n = 10000;

  std::unique_ptr<GenericDataTable<ValueT>> data_table(
    new GenericDataTable<ValueT>(max_key_size));
  std::unique_ptr<BaseGenericIndex<ValueT>> data_index(
    create_generic_index<ValueT>(index_type, data_table.get()));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  std::map<GenericKey, std::pair<Uint64, ValueT>> validation_set;
  
  FastRandom rand;

  size_t key_size = max_key_size - 1;

  GenericKey key(key_size);

  // insert
  for (size_t i = 0; i < n; ++i) {

    rand.next_readable_chars(key_size, key.raw());

    ValueT value = i + 2048;

    OffsetT offset = data_table->insert_tuple(key.raw(), key_size, value);

    validation_set.insert(
      std::pair<GenericKey, std::pair<Uint64, ValueT>>(
        key, std::pair<Uint64, ValueT>(offset.raw_data(), value)));

    // data_index->insert(key.raw(), key_size, value);
  }

  // find
  // for (auto entry : validation_set) {
  //   GenericKey key = entry.first;

  //   std::vector<Uint64> offsets;

  //   data_index->find(key.raw(), key.size(), offsets);

  //   // EXPECT_EQ(offsets.size(), 1);

  //   // ValueT *value = data_table->get_tuple_value(offsets.at(0));

  //   // EXPECT_EQ(offsets.at(0), entry.second.first);

  //   // EXPECT_EQ(*value, entry.second.second);
  // }
}


TEST_F(DynamicIndexGenericTest, UniqueKeyFindTest) {
  
  std::vector<IndexType> index_types {

    // dynamic indexes - singlethread
    IndexType::D_ST_StxBtree,
    // IndexType::D_ST_ArtTree,
    
    // // dynamic indexes - multithread
    // IndexType::D_MT_Libcuckoo,
    // IndexType::D_MT_ArtTree,
    // IndexType::D_MT_BwTree,
    // IndexType::D_MT_Masstree,
  };

  for (auto index_type : index_types) {
    test_dynamic_index_generic_unique_key_find<uint64_t>(32, index_type);

    test_dynamic_index_generic_unique_key_find<uint64_t>(64, index_type);
  }


}
