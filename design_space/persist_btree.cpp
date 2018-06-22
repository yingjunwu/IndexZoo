#include <iostream>

#include "persist_btree.h"

int main() {

  Storage storage;
  PersistVector<uint64_t> vec(&storage);
  for (size_t i = 100; i > 0; --i) {
    vec.insert(i, i + 1024);
  }

  vec.persist();

  vec.print();

  vec.cache();

  vec.print();

  std::vector<uint64_t> values;

  vec.find(99, values);

  for (auto entry : values) {
    std::cout << entry << std::endl;
  }

}
