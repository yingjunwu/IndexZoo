#include <map>
#include <unordered_map>
#include <vector>

#include "harness.h"
#include "fast_random.h"
#include "time_measurer.h"

#include "data_table.h"

#include "index_all.h"


class IndexTest : public LearnedIndexTest {};

typedef uint64_t ValueT;


TEST_F(IndexTest, UniqueKeyTest) {

  std::vector<IndexType> index_types {
    // IndexType::LibcuckooIndexType,
    // IndexType::ArtTreeSTIndexType,
    // IndexType::ArtTreeMTIndexType,
    IndexType::SinglethreadDynamicStxBtreeIndexType,
    // IndexType::BwTreeIndexType,
  };

  size_t n = 10000;

  for (auto index_type : index_types) {

    std::unique_ptr<DataTable<uint64_t, uint64_t>> data_table(
      new DataTable<uint64_t, uint64_t>());
    std::unique_ptr<BaseIndex<uint64_t>> data_index(
      create_index<uint64_t>(index_type, data_table.get()));

    data_index->prepare_threads(1);
    data_index->register_thread(0);

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

// TEST_F(IndexTest, DuplicateKeyTest) {

//   std::vector<IndexType> index_types {
//     IndexType::LibcuckooIndexType,
//     // IndexType::ArtTreeSTIndexType,
//     IndexType::ArtTreeMTIndexType,
//     IndexType::StxBtreeIndexType,
//     IndexType::BwTreeIndexType,
//   };

//   size_t n = 10000;
//   size_t m = 1000;

//   for (auto index_type : index_types) {

//     FastRandom rand_gen(0);

//     std::unique_ptr<DataTable<uint64_t, uint64_t>> data_table(
//       new DataTable<uint64_t, uint64_t>());
//     std::unique_ptr<BaseIndex<uint64_t>> data_index(
//       create_index<uint64_t>(index_type, data_table.get()));

//     data_index->prepare_threads(1);
//     data_index->register_thread(0);

//     std::multimap<uint64_t, std::pair<Uint64, uint64_t>> validation_set;
    
//     // insert
//     for (size_t i = 0; i < n; ++i) {

//       uint64_t key = rand_gen.next() / m;
//       uint64_t value = i + 2048;
      
//       OffsetT offset = data_table->insert_tuple(key, value);
      
//       validation_set.insert(
//         std::pair<uint64_t, std::pair<Uint64, uint64_t>>(
//           key, std::pair<Uint64, uint64_t>(offset.raw_data(), value)));

//       data_index->insert(key, offset.raw_data());
//     }

//     // find
//     for (size_t i = 0; i < m; ++i) {
//       uint64_t key = i;

//       std::vector<Uint64> offsets;

//       data_index->find(key, offsets);

//       EXPECT_EQ(offsets.size(), 1);

//       uint64_t *value = data_table->get_tuple_value(offsets.at(0));

//       EXPECT_EQ(offsets.at(0), validation_set.find(key)->second.first);

//       EXPECT_EQ(*value, validation_set.find(key)->second.second);
//     }
//   }

// }

// TEST_F(IndexTest, RangeFindTest) {

//   std::vector<IndexType> index_types {
//     IndexType::LibcuckooIndexType,
//     IndexType::ArtTreeSTIndexType,
//     IndexType::ArtTreeMTIndexType,
//     IndexType::StxBtreeIndexType,
//     IndexType::BwTreeIndexType,
//   };

//   for (auto index_type : index_types) {

//     std::unique_ptr<DataTable<uint64_t, uint64_t>> data_table(
//       new DataTable<uint64_t, uint64_t>());
//     std::unique_ptr<BaseIndex<uint64_t>> data_index(
//       create_index<uint64_t>(index_type, data_table.get()));

//     data_index->prepare_threads(1);
//     data_index->register_thread(0);

//     std::multimap<uint64_t, std::pair<Uint64, uint64_t>> validation_set;
    
//     // insert
//     for (size_t i = 0; i < 10000; ++i) {

//       uint64_t key = i;
//       uint64_t value = i + 2048;
      
//       OffsetT offset = data_table->insert_tuple(key, value);
      
//       validation_set.insert(
//         std::pair<uint64_t, std::pair<Uint64, uint64_t>>(
//           key, std::pair<Uint64, uint64_t>(offset.raw_data(), value)));

//       data_index->insert(key, offset.raw_data());
//     }

//     // find
//     for (size_t i = 0; i < 100; ++i) {
//       uint64_t key = i;

//       std::vector<Uint64> offsets;

//       data_index->find(key, offsets);

//       EXPECT_EQ(offsets.size(), 1);

//       uint64_t *value = data_table->get_tuple_value(offsets.at(0));

//       EXPECT_EQ(offsets.at(0), validation_set.find(key)->second.first);

//       EXPECT_EQ(*value, validation_set.find(key)->second.second);
//     }
//   }

// }









// TEST_F(IndexTest, UniqueKeyTest) {

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

//    unsigned n=(1<<(16+(K*4)));

//    for (unsigned i=0;i<K;i++)
//       scale+=pow16(i);
//    scale*=16;

//    int32_t* keys=(int32_t*)malloc_huge(sizeof(int32_t)*(n));
//    LeafEntry* leaves=new LeafEntry[n];
//    for (unsigned i=0;i<n;i++) {
//       keys[i]=i;
//       leaves[i].value=i;
//       leaves[i].key=i;
//    }

//    int32_t* fast=buildFAST(leaves,n);
//    std::random_shuffle(keys,keys+n);
//    unsigned repeat=n>10000000 ? 1 : 10000000/n;

//    double start=gettime();
//    for (unsigned r=0; r<repeat; r++)
//       for (unsigned i=0;i<n;i++)
//          assert(search(fast,keys[i])+1);
//    std::cout << "searchnoleaf:" << (n*repeat/1000000.0)/(gettime()-start) << std::endl;

//    start=gettime();
//    for (unsigned r=0; r<repeat; r++)
//       for (unsigned i=0;i<n;i++)
//          assert(leaves[search(fast,keys[i])].key==keys[i]);
//    std::cout << "search:" << (n*repeat/1000000.0)/(gettime()-start) << std::endl;

// }

