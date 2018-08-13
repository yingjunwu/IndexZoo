#include <iostream>

#include "fast_random.h"
#include "time_measurer.h"
#include "data_table.h"
#include "index_all.h"


template<typename ValueT>
void scan_performance(const size_t max_key_size, const IndexType index_type) {

  size_t n = 10000000;

  std::unique_ptr<GenericDataTable<ValueT>> data_table(
    new GenericDataTable<ValueT>(max_key_size));
  std::unique_ptr<BaseGenericIndex<ValueT>> data_index(
    create_generic_index<ValueT>(index_type, data_table.get()));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  FastRandom rand_gen(0);

  size_t key_size = max_key_size - 1;
  GenericKey key(key_size);

  // insert
  for (size_t i = 0; i < n; ++i) {

    rand_gen.next_readable_chars(key_size, key.raw());
    ValueT value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key.raw(), key.size(), value);
    data_index->insert(key, offset.raw_data());
  }

  TimeMeasurer timer;
  timer.tic();

  std::vector<Uint64> offsets;
  data_index->scan_full(offsets, n / 2);

  timer.toc();
  timer.print_ms();
  
}

int main() {

  std::vector<IndexType> index_types {
    IndexType::D_ST_StxBtree,
    IndexType::D_ST_ArtTree,
  };

  for (auto index_type : index_types) {
    scan_performance<uint64_t>(8, index_type);
  }
}
