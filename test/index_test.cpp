#include <map>
#include <unordered_map>
#include <vector>

#include "harness.h"
#include "fast_random.h"
#include "time_measurer.h"

#include "data_table.h"

#include "index_all.h"


class IndexTest : public LearnedIndexTest {};


TEST_F(IndexTest, UniqueKeyTest) {

  // size_t n = 1000;

  // std::unique_ptr<DataTable<uint64_t, uint64_t>> data_table(
  //   new DataTable<uint64_t, uint64_t>());
  // std::unique_ptr<BaseIndex<uint64_t>> data_index(
  //   new BtreeIndex<uint64_t>());

  // std::unordered_map<uint64_t, std::pair<Uint64, uint64_t>> validation_set;
  
  // // insert
  // for (size_t i = 0; i < n; ++i) {

  //   uint64_t key = n - i - 1;
  //   uint64_t value = i + 2048;
    
  //   OffsetT offset = data_table->insert_tuple(key, value);
    
  //   validation_set.insert(
  //     std::pair<uint64_t, std::pair<Uint64, uint64_t>>(
  //       key, std::pair<Uint64, uint64_t>(offset.raw_data(), value)));

  //   data_index->insert(key, offset.raw_data());
  // }
  // data_index->print();

  // // reorganize data
  // data_index->reorganize();

  // // find
  // for (size_t i = 0; i < n; ++i) {
  //   uint64_t key = i;

  //   std::vector<Uint64> offsets;

  //   data_index->find(key, offsets);

  //   EXPECT_EQ(offsets.size(), 1);

  //   uint64_t *value = data_table->get_tuple_value(offsets.at(0));

  //   EXPECT_EQ(offsets.at(0), validation_set.find(key)->second.first);

  //   EXPECT_EQ(*value, validation_set.find(key)->second.second);
  // }

   unsigned n=(1<<(16+(K*4)));

   for (unsigned i=0;i<K;i++)
      scale+=pow16(i);
   scale*=16;

   int32_t* keys=(int32_t*)malloc_huge(sizeof(int32_t)*(n));
   LeafEntry* leaves=new LeafEntry[n];
   for (unsigned i=0;i<n;i++) {
      keys[i]=i;
      leaves[i].value=i;
      leaves[i].key=i;
   }

   int32_t* fast=buildFAST(leaves,n);
   std::random_shuffle(keys,keys+n);
   unsigned repeat=n>10000000 ? 1 : 10000000/n;

   double start=gettime();
   for (unsigned r=0; r<repeat; r++)
      for (unsigned i=0;i<n;i++)
         assert(search(fast,keys[i])+1);
   std::cout << "searchnoleaf:" << (n*repeat/1000000.0)/(gettime()-start) << std::endl;

   start=gettime();
   for (unsigned r=0; r<repeat; r++)
      for (unsigned i=0;i<n;i++)
         assert(leaves[search(fast,keys[i])].key==keys[i]);
   std::cout << "search:" << (n*repeat/1000000.0)/(gettime()-start) << std::endl;

}

