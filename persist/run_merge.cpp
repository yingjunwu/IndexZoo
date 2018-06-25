#include "hash_run.h"
#include "hash_run_merger.h"

#include "synopsis_run.h"
#include "synopsis_run_merger.h"

#include "time_measurer.h"
#include "fast_random.h"


template<typename KeyType, typename RunType, typename RunMergerType>
void do_merge() {

  TimeMeasurer timer;
  FastRandom rand;
  
  RunType run0("run_0");
  for (size_t i = 0; i < 1000; ++i) {
    run0.insert(rand.next<KeyType>() % (1024 * 1024), i);
  }
  run0.persist();

  RunType run1("run_1");
  for (size_t i = 0; i < 1000; ++i) {
    run1.insert(rand.next<KeyType>() % (1024 * 1024), i);
  }
  run1.persist();

  RunMergerType merge_run("run_m");
  merge_run.merge(&run0, &run1);
  merge_run.print();

  std::vector<uint64_t> result;
  merge_run.find(326, result);

  for (auto entry : result) {
    std::cout << "result: " << entry << std::endl;
  }
}

int main() {
  do_merge<uint64_t, HashRun<uint64_t>, HashRunMerger<uint64_t>>();
  do_merge<uint64_t, SynopsisRun<uint64_t>, SynopsisRunMerger<uint64_t>>();

}
