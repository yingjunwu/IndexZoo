#include <iostream>

#include "persist_btree.h"
#include "time_measurer.h"


int main() {

  TimeMeasurer timer;

  Storage storage;
  PersistVector<uint64_t> vec(&storage);
  for (size_t i = 1000; i > 0; --i) {
    vec.insert(i, i + 1024);
  }

  vec.persist();

  vec.print();

  vec.cache();

  vec.print();

  std::vector<uint64_t> values;

  timer.tic();

  vec.find(599, values);

  timer.toc();

  timer.print_us();

  for (auto entry : values) {
    std::cout << entry << std::endl;
  }

}
