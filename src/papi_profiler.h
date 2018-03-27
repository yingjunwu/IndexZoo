#include "papi.h"

class PAPIProfiler {

public:

  static void init_papi() {
    int retval;

    retval = PAPI_library_init(PAPI_VER_CURRENT);
    if(retval != PAPI_VER_CURRENT && retval > 0) {
      fprintf(stderr,"PAPI library version mismatch!\n");
      exit(1); 
    }

    if (retval < 0) {
      fprintf(stderr, "PAPI failed to start (1): %s\n", PAPI_strerror(retval));
      exit(1);
    }

    retval = PAPI_is_initialized();

    if (retval != PAPI_LOW_LEVEL_INITED) {
      fprintf(stderr, "PAPI failed to start (2): %s\n", PAPI_strerror(retval));
      exit(1);
    }

    return;
  }

  static void start_measure_cache_miss_rate() {
    const size_t num_counters = 3;
    int events[num_counters] = {PAPI_L1_TCM, PAPI_LD_INS, PAPI_SR_INS};
    long long counters[num_counters];
    int retval;

    if ((retval = PAPI_start_counters(events, num_counters)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to start counters: %s\n", PAPI_strerror(retval));
      exit(EXIT_FAILURE);
    }
    return;
  }

  static void stop_measure_cache_miss_rate() {
    const size_t num_counters = 3;
    long long counters[num_counters];
    int retval;

    if ((retval = PAPI_stop_counters(counters, num_counters)) != PAPI_OK) {
      fprintf(stderr, "PAPI failed to stop counters: %s\n", PAPI_strerror(retval));
      exit(EXIT_FAILURE);
    }

    std::cout << "counters: " << counters[0] << " " << counters[1] << " " << counters[2] << std::endl;
    std::cout << "cache miss rate = " << counters[0] * 1.0 / (counters[1] + counters[2]) << std::endl;
    return;
  }

};