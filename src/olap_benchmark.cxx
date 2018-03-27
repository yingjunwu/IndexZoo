#include <cassert>
#include <cstdint>
#include <vector>
#include <thread>
#include <cstdio>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <getopt.h>

#include "time_measurer.h"
#include "data_table.h"
#include "index_all.h"
#include "key_generator_all.h"
// #include "papi_profiler.h"


void usage(FILE *out) {
  fprintf(out,
          "Command line options : olap_benchmark <options> \n"
          "   -h --help              :  print help message \n"
          "   -i --index             :  index type: \n"
          "                              -- (0) interpolation index (default) \n"
          "                              -- (1) binary index \n"
          "                              -- (2) kary index \n"
          "                              -- (3) fast index \n"
          "   -S --index_param_1     :  1st index parameter \n"
          "   -T --index_param_2     :  2nd index parameter \n"
          "   -y --read_type         :  read type: \n"
          "                              -- (0) index lookup (default) \n"
          "                              -- (1) index scan \n"
          "                              -- (2) index reverse scan \n"
          "   -t --time_duration     :  time duration (default: 10) \n"
          "   -m --key_count         :  key count (default: 1ull<<20) \n"
          "   -r --reader_count      :  reader count (default: 1) \n"
          "   -d --distribution      :  data distribution: \n"
          "                              -- (0) sequence (default) \n"
          "                              -- (1) uniform distribution \n"
          "                              -- (2) normal distribution \n"
          "                              -- (3) log-normal distribution \n"
          "   -u --key_upper_bound   :  key upper bound \n"
          "   -P --dist_param_1      :  1st distribution parameter \n"
          "   -Q --dist_param_2      :  2nd distribution parameter \n"
  );
}

static struct option opts[] = {
    { "index",             optional_argument, NULL, 'i' },
    { "read_type",         optional_argument, NULL, 'y' },
    { "index_param_1",     optional_argument, NULL, 'S' },
    { "index_param_2",     optional_argument, NULL, 'T' },
    { "time_duration",     optional_argument, NULL, 't' },
    { "key_count",         optional_argument, NULL, 'm' },
    { "reader_count",      optional_argument, NULL, 'r' },
    { "distribution",      optional_argument, NULL, 'd' },
    { "key_upper_bound",   optional_argument, NULL, 'u' },
    { "dist_param_1",      optional_argument, NULL, 'P' },
    { "dist_param_2",      optional_argument, NULL, 'Q' },
    { NULL, 0, NULL, 0 }
};

enum class ReadType {
  IndexLookupType = 0,
  IndexScanType,
  IndexScanReverseType,
};

struct Config {
  StaticIndexType index_type_ = StaticIndexType::InterpolationIndexType;
  ReadType index_read_type_ = ReadType::IndexLookupType;
  DistributionType distribution_type_ = DistributionType::SequenceType;
  size_t index_param_1_ = 1;
  size_t index_param_2_ = 2;
  uint64_t key_upper_bound_ = INVALID_KEY_BOUND;
  double dist_param_1_ = INVALID_DIST_PARAM;
  double dist_param_2_ = INVALID_DIST_PARAM;
  uint64_t time_duration_ = 10;
  double profile_duration_ = 0.5;
  uint64_t key_count_ = 1ull << 20;
  uint64_t reader_count_ = 1;
};

void validate_key_generator_params(const Config &config);

void parse_args(int argc, char* argv[], Config &config) {
  
  while (1) {
    int idx = 0;
    int c = getopt_long(argc, argv, "ht:m:r:i:S:T:y:d:u:P:Q:", opts, &idx);

    if (c == -1) break;

    switch (c) {
      case 'i': {
        config.index_type_ = (StaticIndexType)atoi(optarg);
        break;
      }
      case 'S': {
        config.index_param_1_ = atoi(optarg);
        break;
      }
      case 'T': {
        config.index_param_2_ = atoi(optarg);
        break;
      }
      case 'y': {
        config.index_read_type_ = (ReadType)atoi(optarg);
        break;
      }
      case 'd': {
        config.distribution_type_ = (DistributionType)atoi(optarg);
        break;
      }
      case 't': {
        config.time_duration_ = (uint64_t)atoi(optarg);
        break;
      }
      case 'm': {
        config.key_count_ = (uint64_t)atoi(optarg);
        break;
      }
      case 'r': {
        config.reader_count_ = (uint64_t)atoi(optarg);
        break;
      }
      case 'u': {
        config.key_upper_bound_ = (uint64_t)atoi(optarg);
        break;
      }
      case 'P': {
        config.dist_param_1_ = (double)atof(optarg);
        break;
      }
      case 'Q': {
        config.dist_param_2_ = (double)atof(optarg);
        break;
      }
      case 'h': {
        usage(stderr);
        exit(EXIT_FAILURE);
        break;
      }
      default: {
        fprintf(stderr, "Unknown option: -%c-\n", c);
        usage(stderr);
        exit(EXIT_FAILURE);
        break;
      }
    }
  }

  validate_static_index_params(config.index_type_, config.index_param_1_, config.index_param_2_);

  validate_key_generator_params(config.distribution_type_, config.key_upper_bound_, config.dist_param_1_, config.dist_param_2_);

  std::cout << ">>>>>>>>>>>>>>>" << std::endl;

}

bool is_running = false;
uint64_t *operation_counts = nullptr;

typedef Uint64 KeyT;
typedef Uint64 ValueT;

// table and index
std::unique_ptr<DataTable<KeyT, ValueT>> data_table(nullptr);
std::unique_ptr<BaseStaticIndex<KeyT, ValueT>> data_index(nullptr);

void run_reader_thread(const uint64_t &thread_id, const Config &config) {

  pin_to_core(thread_id);

  std::unique_ptr<BaseKeyGenerator> key_generator(construct_key_generator(config.distribution_type_, thread_id, config.key_upper_bound_, config.dist_param_1_, config.dist_param_2_));

  uint64_t &operation_count = operation_counts[thread_id];
  operation_count = 0;

  if (config.index_read_type_ == ReadType::IndexLookupType) {

    while (true) {
      if (is_running == false) {
        break;
      }

      KeyT key = key_generator->get_read_key();
      
      std::vector<Uint64> values;

      data_index->find(key, values);
      
      ++operation_count;
    }

  } else if (config.index_read_type_ == ReadType::IndexScanType) {

    while (true) {
      if (is_running == false) {
        break;
      }

      KeyT key = key_generator->get_read_key();
      
      std::vector<Uint64> values;

      data_index->scan(key, values);
      
      ++operation_count;
    }

  } else {
    ASSERT(config.index_read_type_ == ReadType::IndexScanReverseType, "invalid index read type");

    while (true) {
      if (is_running == false) {
        break;
      }

      KeyT key = key_generator->get_read_key();
      
      std::vector<Uint64> values;

      data_index->scan_reverse(key, values);
      
      ++operation_count;
    }

  }
}


void run_workload(const Config &config) {

  std::unique_ptr<BaseKeyGenerator> key_generator(construct_key_generator(config.distribution_type_, 0, config.key_upper_bound_, config.dist_param_1_, config.dist_param_2_));

  for (size_t i = 0; i < config.key_count_; ++i) {

    KeyT key = key_generator->get_insert_key();
    ValueT value = 100;
    
    OffsetT offset = data_table->insert_tuple(key, value);

    // ATTENTION: WE DO NOT INSERT INTO INDEX DIRECTLY!
  }

  data_index->reorganize();

  operation_counts = new uint64_t[config.reader_count_];
  uint64_t profile_round = (uint64_t)(config.time_duration_ / config.profile_duration_);

  uint64_t **operation_counts_profiles = new uint64_t*[profile_round];
  for (uint64_t round_id = 0; round_id < profile_round; ++round_id) {
    operation_counts_profiles[round_id] = new uint64_t[config.reader_count_];
    memset(operation_counts_profiles[round_id], 0, config.reader_count_ * sizeof(uint64_t));
  }
  std::vector<double> act_size_profiles; // actual allocated size. Unit: GB.
  std::vector<size_t> approx_size_profiles; // approximate data size. Unit: #tuples.

  std::vector<uint64_t> read_counts; // number of read operations performed.

  double init_mem_size = get_memory_mb();
  std::cout << "init memory size: " << init_mem_size << " MB" << std::endl;
  
  // launch a group of threads
  is_running = true;
  std::vector<std::thread> worker_threads;
  uint64_t thread_count = 0;
  

  // PAPIProfiler::init_papi();
  // PAPIProfiler::start_measure_cache_miss_rate();
  
  // reader threads
  for (; thread_count < config.reader_count_; ++thread_count) {
    worker_threads.push_back(std::move(std::thread(run_reader_thread, thread_count, config)));
  }

  std::cout << "        TIME         READ       RAM (act.)   RAM (est.)" << std::endl;

  for (uint64_t round_id = 0; round_id < profile_round; ++round_id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(int(config.profile_duration_ * 1000)));
    
    memcpy(operation_counts_profiles[round_id], operation_counts, sizeof(uint64_t) * config.reader_count_);

    act_size_profiles.push_back(get_memory_mb());
    approx_size_profiles.push_back(data_table->size_approx());
    if (round_id == 0) {
      // first round
      uint64_t read_count = 0;

      uint64_t thread_count = 0;

      // count reads
      for (; thread_count < config.reader_count_; ++thread_count) {
        read_count += operation_counts_profiles[0][thread_count];
      }
      read_counts.push_back(read_count);

    } else {
      // remaining rounds
      uint64_t read_count = 0;

      uint64_t thread_count = 0;
      
      // count reads
      for (; thread_count < config.reader_count_; ++thread_count) {
        read_count += operation_counts_profiles[round_id][thread_count] - operation_counts_profiles[round_id - 1][thread_count];
      }
      read_counts.push_back(read_count);
    }

    // print out
    std::cout << std::fixed << std::setprecision(2) << std::right
              << "[" 
              << std::setw(5) 
              << config.profile_duration_ * round_id << " - " 
              << std::setw(5)
              << config.profile_duration_ * (round_id + 1) 
              << " s]:  ";

    if (read_counts.at(round_id) * 1.0 / 1000 / 1000 < 0.1) {
      std::cout << std::setw(5)
              << read_counts.at(round_id) * 1.0 / 1000 
              << " K  |  "; 
    } else {
      std::cout << std::setw(5)
              << read_counts.at(round_id) * 1.0 / 1000 / 1000 
              << " M  |  "; 
    } 
    std::cout << std::setw(5)
              << act_size_profiles.at(round_id) 
              << " MB  |  "
              << std::setw(5)
              << approx_size_profiles.at(round_id) * (sizeof(KeyT) + sizeof(ValueT)) * 1.0 / 1024 / 1024
              << " MB"
              << std::endl;
  }
  
  // join all the threads
  is_running = false;

  for (uint64_t i = 0; i < config.reader_count_; ++i) {
    worker_threads.at(i).join();
  }

  // PAPIProfiler::stop_measure_cache_miss_rate();
  
  uint64_t total_count = 0;
  for (uint64_t i = 0; i < config.reader_count_; ++i) {
    total_count += operation_counts[i];
  }

  std::cout << "average throughput: " << total_count * 1.0 / config.time_duration_ / 1000 / 1000 << " M ops" 
            << std::endl;

  data_index->print_stats();

  for (uint64_t round_id = 0; round_id < profile_round; ++round_id) {
    delete[] operation_counts_profiles[round_id];
    operation_counts_profiles[round_id] = nullptr;
  }

  delete[] operation_counts_profiles;
  operation_counts_profiles = nullptr;

  delete[] operation_counts;
  operation_counts = nullptr;
}

int main(int argc, char* argv[]) {

  Config config;

  parse_args(argc, argv, config);

  data_table.reset(new DataTable<KeyT, ValueT>());

  data_index.reset(create_static_index<KeyT, ValueT>(config.index_type_, data_table.get(), config.index_param_1_, config.index_param_2_));

  run_workload(config);
  
}
