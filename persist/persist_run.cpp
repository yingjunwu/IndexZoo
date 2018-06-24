#include "base_run.h"
#include "hash_run.h"
#include "synopsis_run.h"
#include "time_measurer.h"


int main() {

  TimeMeasurer timer;

  HashRun<uint64_t> run(0);
  for (size_t i = 1000; i > 0; --i) {
    run.insert(i, i + 1024);
  }

  run.persist();

  run.print();

  run.cache();

  run.print();

  std::vector<uint64_t> values;

  timer.tic();

  run.find(599, values);

  timer.toc();

  timer.print_us();

  for (auto entry : values) {
    std::cout << entry << std::endl;
  }

}
