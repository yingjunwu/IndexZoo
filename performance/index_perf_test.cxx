#include <iostream>

#include "fast_random.h"
#include "time_measurer.h"
#include "data_table.h"
#include "index_all.h"


template<typename KeyT, typename ValueT>
void scan_performance(const IndexType index_type) {

  size_t n = 10000000;

  std::unique_ptr<DataTable<KeyT, ValueT>> data_table(
    new DataTable<KeyT, ValueT>());
  std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(
    create_numeric_index<KeyT, ValueT>(index_type, data_table.get(), 2, INVALID_INDEX_PARAM));

  data_index->prepare_threads(1);
  data_index->register_thread(0);

  FastRandom rand_gen(0);

  // insert
  for (size_t i = 0; i < n; ++i) {

    KeyT key = rand_gen.next<KeyT>();
    // KeyT key = i;
    ValueT value = i + 2048;
    
    OffsetT offset = data_table->insert_tuple(key, value);
    data_index->insert(key, offset.raw_data());
  }

  data_index->reorganize();


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
    IndexType::S_Interpolation,
  };

  for (auto index_type : index_types) {
    scan_performance<uint32_t, uint64_t>(index_type);
  }
}
