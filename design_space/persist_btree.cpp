#include <iostream>

#include "persist_btree.h"

int main() {
  Storage storage;
  char *block = new char[BLOCK_SIZE];
  uint64_t i0 = 100;
  uint64_t i1 = 110;
  uint64_t offset = 0;
  memcpy(block, &i0, sizeof(uint64_t));
  memcpy(block + 8, &i1, sizeof(uint64_t));

  size_t block_id = storage.write_block(block);

  uint64_t i2 = 120;
  memcpy(block, &i2, sizeof(uint64_t));

  block_id = storage.write_block(block);

  char *new_block = new char[BLOCK_SIZE];
  storage.read_data(block_id, new_block);
  uint64_t ret = 1111;
  memcpy(&ret, new_block, sizeof(uint64_t));
  std::cout << "ret = " << ret << std::endl;

}
