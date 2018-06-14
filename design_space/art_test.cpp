#include "art.h"

int main() {

  art_tree tree;
  art_tree_init(&tree);

  // for (uint64_t i = 0; i < 100; ++i) {
  //   uint64_t bs_key = __builtin_bswap64(i);
  //   uint64_t value = i;
  //   art_insert(&tree, (unsigned char*)(&bs_key), sizeof(uint64_t), value); 
  // }

  unsigned char a[16] = "abcdefghijklmno";
  unsigned char b[16] = "abcdefghijklmnp";
  int64_t value1 = 123;
  int64_t value2 = 456;
  art_insert(&tree, a, 15, value1);
  art_insert(&tree, b, 15, value2);

  std::vector<uint64_t> rets;
  art_search(&tree, a, 15, rets);
  std::cout << "rets size = " << rets.size() << std::endl;

  // std::vector<uint64_t> rets;
  // art_scan(&tree, rets);
  // for (size_t i = 0; i < rets.size(); ++i) {
  //   std::cout << rets[i] << std::endl;
  // }

  art_tree_destroy(&tree);

}