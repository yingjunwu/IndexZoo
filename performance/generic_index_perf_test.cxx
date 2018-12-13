#include <iostream>
#include <map>
#include <unordered_map>


#include "fast_random.h"
#include "time_measurer.h"
#include "data_table.h"
#include "index_all.h"


void lookup_performance(const size_t max_key_size, const IndexType index_type) {

  size_t n = 10000;

  std::unique_ptr<GenericDataTable> data_table(
    new GenericDataTable(max_key_size, sizeof(uint64_t)));
  std::unique_ptr<BaseGenericIndex> data_index(
    create_generic_index(index_type, data_table.get()));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  std::map<GenericKey, std::pair<Uint64, uint64_t>> validation_set;

  FastRandom rand_gen(0);

  GenericKey key(max_key_size);

  // insert
  for (size_t i = 0; i < n; ++i) {

    rand_gen.next_readable_chars(max_key_size, key.raw());
    uint64_t value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key.raw(), key.size(), (char*)(&value), sizeof(uint64_t));

    validation_set.insert( {key, {offset.raw_data(), value}} );

    data_index->insert(key, offset.raw_data());
  }

  TimeMeasurer timer;
  timer.tic();

  for (auto &entry : validation_set) {
    GenericKey key = entry.first;

    std::vector<Uint64> offsets;

    data_index->find(key, offsets);

  }

  timer.toc();
  timer.print_ms();
  
}

int main() {

  std::vector<IndexType> index_types {
    IndexType::D_ST_StxBtree,
    IndexType::D_ST_ArtTree,
    // IndexType::D_ST_SdTree,
    IndexType::D_MT_BwTree,
  };

  for (auto index_type : index_types) {
    lookup_performance(8, index_type);
  }
}
