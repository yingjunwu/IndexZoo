#include "base_run.h"
#include "hash_run.h"
#include "synopsis_run.h"
#include "time_measurer.h"
#include "fast_random.h"


int main() {

  TimeMeasurer timer;
  FastRandom rand;
  
  // LogicalHashRun<uint64_t> run0(0);
  // for (size_t i = 0; i < 1000; ++i) {
  //   run0.insert(rand.next<uint64_t>() % (1024 * 1024), i);
  // }
  // run0.persist();

  // LogicalHashRun<uint64_t> run1(1);
  // for (size_t i = 0; i < 1000; ++i) {
  //   run1.insert(rand.next<uint64_t>() % (1024 * 1024), i);
  // }
  // run1.persist();

  // LogicalHashRun<uint64_t> run2(2);
  // run2.merge(&run0, &run1);

  // run2.cache();

  // run2.print();

}
