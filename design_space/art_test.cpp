#include "art.h"

int main() {

  art_tree tree;
  art_tree_init(&tree);

  for (uint64_t i = 0; i < 1000; ++i) {
    uint64_t bs_key = __builtin_bswap64(i);
    uint64_t value = i;
    art_insert(&tree, (unsigned char*)(&bs_key), sizeof(uint64_t), value); 
  }

  uint64_t lower = 10;
  uint64_t higher = 20;
  uint64_t bs_lower = __builtin_bswap64(lower);
  uint64_t bs_higher = __builtin_bswap64(higher);
  std::vector<uint64_t> rets;
  art_range_scan(&tree, (unsigned char*)(&bs_lower), sizeof(uint64_t), (unsigned char*)(&bs_higher), sizeof(uint64_t), rets);
  std::cout << "rets size = " << rets.size() << std::endl;

  // unsigned char a[16] = "012345678901234";
  // unsigned char b[16] = "012345678901235";
  // int64_t value1 = 123;
  // int64_t value2 = 456;
  // art_insert(&tree, a, 15, value1);
  // art_insert(&tree, b, 15, value2);

  // std::vector<uint64_t> rets;
  // unsigned char c[16] = "012345678902234";
  // art_search(&tree, c, 15, rets);
  // std::cout << "rets size = " << rets.size() << std::endl;

  // std::vector<uint64_t> rets;
  // art_scan(&tree, rets);
  // for (size_t i = 0; i < rets.size(); ++i) {
  //   std::cout << rets[i] << std::endl;
  // }

  art_tree_destroy(&tree);

}