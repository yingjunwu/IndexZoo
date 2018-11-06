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
#include "generic_data_table.h"
#include "index_all.h"
#include "generic_key_generator_all.h"
// #include "papi_profiler.h"


void usage(FILE *out) {
  fprintf(out,
          "Command line options : index_benchmark <options> \n"
          "   -h --help              :  print help message \n"
          // index structure
          "   -i --index             :  index type: \n"
          "                              --  (0) dynamic - singlethread - stx-btree index (default)  \n"
          "                              --  (1) dynamic - singlethread - art-tree index \n"
          "                              -- (10) dynamic - multithread  - libcuckoo index \n"
          "                              -- (11) dynamic - multithread  - art-tree index \n"
          "                              -- (12) dynamic - multithread  - bw-tree index \n"
          "                              -- (13) dynamic - multithread  - masstree index \n"
          "   -k --key_size          :  index max key size (default: 8 bytes) \n"
          // configuration
          "   -t --time_duration     :  time duration (default: 10) \n"
          "   -y --read_type         :  read type: \n"
          "                              -- (0) index lookup (default) \n"
          "                              -- (1) index scan \n"
          "                              -- (2) index reverse scan \n"
          "   -r --read_ratio        :  read ratio (default: 1.0) \n"
          "   -s --thread_count      :  thread count (default: 1) \n"
          "   -m --key_count         :  key count (default: 1ull<<20) \n"
          "   -w --workload          :  workload type: \n"
          "                              -- (0) synthetic (default) \n"
          "                              -- (1) username \n"
          "                              -- (2) url \n"
          "   -c --record           :  record all keys \n"
          "   -v --verbose          :  verbose \n"
  );
}

static struct option opts[] = {
    // index structure
    { "index",             optional_argument, NULL, 'i' },
    { "key_size",          optional_argument, NULL, 'k' },
    // configuration
    { "time_duration",     optional_argument, NULL, 't' },
    { "read_type",         optional_argument, NULL, 'y' },
    { "read_ratio",        optional_argument, NULL, 'r' },
    { "thread_count",      optional_argument, NULL, 's' },
    // data distribution
    { "key_count",         optional_argument, NULL, 'm' },
    { "workload",          optional_argument, NULL, 'w' },
    { "record",            optional_argument, NULL, 'c' },
    { "verbose",           optional_argument, NULL, 'v' },
    { NULL, 0, NULL, 0 }
};

enum class ReadType {
  IndexLookupType = 0,
  IndexScanType,
  IndexScanReverseType,
};

struct Config {
  // index structure
  IndexType index_type_ = IndexType::D_ST_StxBtree;
  int key_size_ = 8; // unit: bytes
  int value_size_ = 8; // unit: bytes
  // configuration
  const double profile_duration_ = 0.5; // fixed
  int time_duration_ = 10;
  ReadType index_read_type_ = ReadType::IndexLookupType;
  double read_ratio_ = 1.0;
  int thread_count_ = 1;
  // data distribution
  uint64_t key_count_ = 1ull << 20;
  WorkloadType workload_type_ = WorkloadType::SyntheticType;
  bool record_ = false;
  bool verbose_ = false;

  void print() {
    std::cout << "=====     INDEX STRUCTURE    =====" << std::endl;
    std::cout << "max key size: " << key_size_ << std::endl;
    std::cout << "===== WORKLOAD CONFIGURATION =====" << std::endl;
    std::cout << "read ratio: " << read_ratio_ << std::endl;
    std::cout << "thread count: " << thread_count_ << std::endl;
    std::cout << "=====    DATA DISTRIBUTION   =====" << std::endl;
    std::cout << "key count: " << key_count_ << std::endl;
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>" << std::endl;
  }
};

void parse_args(int argc, char* argv[], Config &config) {
  
  while (1) {
    int idx = 0;
    int c = getopt_long(argc, argv, "hcvi:k:t:y:r:s:m:w:", opts, &idx);

    if (c == -1) break;

    switch (c) {
      case 'i': {
        config.index_type_ = (IndexType)atoi(optarg);
        break;
      }
      case 'k': {
        config.key_size_ = atoi(optarg);
        break;
      }
      case 't': {
        config.time_duration_ = atoi(optarg);
        break;
      }
      case 'y': {
        config.index_read_type_ = (ReadType)atoi(optarg);
        break;
      }
      case 'r': {
        config.read_ratio_ = (double)atof(optarg);
        break;
      }
      case 's': {
        config.thread_count_ = atoi(optarg);
        break;
      }
      case 'm': {
        config.key_count_ = (uint64_t)strtoull(optarg, nullptr, 10); // uint64_t
        break;
      }
      case 'w': {
        config.workload_type_ = (WorkloadType)atoi(optarg);
        break;
      }
      case 'c': {
        config.record_ = true;
        break;
      }
      case 'v': {
        config.verbose_ = true;
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

  config.print();

}

bool is_running = false;
uint64_t *operation_counts = nullptr;

void run_thread(const size_t &thread_id, const Config &config, const GenericKey *query_keys, GenericDataTable *data_table, BaseGenericIndex *data_index) {

  pin_to_core(thread_id);

  data_index->register_thread(thread_id);

  std::unique_ptr<BaseGenericKeyGenerator> key_generator(construct_generic_key_generator(config.workload_type_, thread_id, config.key_size_));

  uint64_t &operation_count = operation_counts[thread_id];
  operation_count = 0;

  FastRandom rand_gen(thread_id);

  GenericKey insert_key;

  ValueT value = 100;

  while (true) {
    if (is_running == false) {
      break;
    }

    double next_rand = rand_gen.next_uniform();

    if (next_rand < config.read_ratio_) {

      std::vector<Uint64> offsets;

      // retrieve tuple locations
      data_index->find(query_keys[rand_gen.next<uint64_t>() % config.key_count_], offsets);

      // ASSERT(offsets.size() == 1, "must be 1! " << key);
    } else {
      // insert
      key_generator->get_next_key(insert_key);
      
      OffsetT offset = data_table->insert_tuple(insert_key.raw(), insert_key.size(), (char*)(&value), sizeof(value));

      // insert tuple locations into index
      data_index->insert(insert_key, offset.raw_data());
    }

    ++operation_count;
  }
}

void run_workload(const Config &config) {

  // create table
  std::unique_ptr<GenericDataTable> data_table(nullptr);
  // note that the max_key_size passed to GenericDataTable must be at least 1 byte
  // larger than the real key_size.
  // this is because some index structures need to access base table to determine
  // key data. 
  data_table.reset(new GenericDataTable(config.key_size_ + 1, config.value_size_));

  // create index
  std::unique_ptr<BaseGenericIndex> data_index(nullptr);
  data_index.reset(create_generic_index(config.index_type_, data_table.get()));

  // prepare threads
  data_index->prepare_threads(config.thread_count_);
  data_index->register_thread(0);

  //=================================
  // populate table
  //=================================
  std::unique_ptr<BaseGenericKeyGenerator> key_generator(construct_generic_key_generator(config.workload_type_, 0, config.key_size_));

  double query_key_size_mb = 0;

  GenericKey *init_keys = new GenericKey[config.key_count_]; // store all init keys

  uint64_t value = 100;

  for (size_t i = 0; i < config.key_count_; ++i) {

    key_generator->get_next_key(init_keys[i]);
    
    OffsetT offset = data_table->insert_tuple(init_keys[i].raw(), init_keys[i].size(), (char*)(&value), sizeof(value));

    data_index->insert(init_keys[i], offset.raw_data());

    query_key_size_mb += init_keys[i].size();

  }
  data_index->reorganize();

  query_key_size_mb = query_key_size_mb * 1.0 / 1024 / 1024;
  //=================================

  //=================================
  // write all init keys to output file
  //=================================
  if (config.record_ == true) {

    std::ofstream record_file;
    record_file.open("data.txt");
    
    for (size_t i = 0; i < config.key_count_; ++i) {
      record_file << std::string(init_keys[i].raw(), init_keys[i].size()) << std::endl;
    }
    record_file.close();

    // return;
  }
  //=================================

  //=================================

  operation_counts = new uint64_t[config.thread_count_];
  uint64_t profile_round = (uint64_t)(config.time_duration_ / config.profile_duration_);

  uint64_t **operation_counts_profiles = new uint64_t*[profile_round];
  for (uint64_t round_id = 0; round_id < profile_round; ++round_id) {
    operation_counts_profiles[round_id] = new uint64_t[config.thread_count_];
    memset(operation_counts_profiles[round_id], 0, config.thread_count_ * sizeof(uint64_t));
  }
  std::vector<double> act_size_profiles; // actual allocated size. Unit: MB. include both index and table
  std::vector<double> table_size_profiles; // table data size. Unit: #tuples.

  std::vector<uint64_t> total_operation_counts; // number of total operations performed.

  double init_mem_size = get_memory_mb();
  std::cout << "init memory size (index + table): " << (init_mem_size - query_key_size_mb) << " MB" << std::endl;
  
  // launch a group of threads
  is_running = true;
  std::vector<std::thread> worker_threads;
  
  // PAPIProfiler::init_papi();
  // PAPIProfiler::start_measure_cache_miss_rate();
  
  for (uint64_t thread_id = 0; thread_id < config.thread_count_; ++thread_id) {
    worker_threads.push_back(std::move(std::thread(run_thread, thread_id, std::ref(config), init_keys, data_table.get(), data_index.get())));
  }

  std::cout << "        TIME       THROUGHPUT   RAM (tot.)   RAM (tab.)" << std::endl;

  for (uint64_t round_id = 0; round_id < profile_round; ++round_id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(int(config.profile_duration_ * 1000)));
    
    memcpy(operation_counts_profiles[round_id], operation_counts, sizeof(uint64_t) * config.thread_count_);

    double table_size_approx = data_table->size_approx() * (config.key_size_ + 1 + config.value_size_) * 1.0 / 1024 / 1024;

    table_size_profiles.push_back(table_size_approx);
    act_size_profiles.push_back(get_memory_mb() - query_key_size_mb);

    if (round_id == 0) {
      // first round
      uint64_t operation_count = 0;
      for (size_t thread_id = 0; thread_id < config.thread_count_; ++thread_id) {
        operation_count += operation_counts_profiles[0][thread_id];
      }
      total_operation_counts.push_back(operation_count);

    } else {
      // remaining rounds
      uint64_t operation_count = 0;
      for (size_t thread_id = 0; thread_id < config.thread_count_; ++thread_id) {
        operation_count += operation_counts_profiles[round_id][thread_id] - operation_counts_profiles[round_id - 1][thread_id];
      }
      total_operation_counts.push_back(operation_count);
    }

    // print out
    std::cout << std::fixed << std::setprecision(2) << std::right
              << "[" 
              << std::setw(5) 
              << config.profile_duration_ * round_id << " - " 
              << std::setw(5)
              << config.profile_duration_ * (round_id + 1) 
              << " s]:  ";
    if (total_operation_counts.at(round_id) * 1.0 / 1000 / 1000 < 0.1) {
      std::cout << std::setw(5)
              << total_operation_counts.at(round_id) * 1.0 / 1000 
              << " K  |  "; 
    } else {
      std::cout << std::setw(5)
              << total_operation_counts.at(round_id) * 1.0 / 1000 / 1000 
              << " M  |  "; 
    } 
    std::cout << std::setw(5)
              << act_size_profiles.at(round_id)
              << " MB  |  "
              << std::setw(5)
              << table_size_profiles.at(round_id)
              << " MB"
              << std::endl;
  }
  
  // join all the threads
  is_running = false;

  for (uint64_t i = 0; i < config.thread_count_; ++i) {
    worker_threads.at(i).join();
  }

  // PAPIProfiler::stop_measure_cache_miss_rate();
  
  uint64_t total_count = 0;
  for (uint64_t i = 0; i < config.thread_count_; ++i) {
    total_count += operation_counts[i];
  }

  std::cout << "average throughput: " << total_count * 1.0 / config.time_duration_ / 1000 / 1000 << " M ops" 
            << std::endl;

  if (config.verbose_ == true) {
    data_index->print(); 
  }

  for (uint64_t round_id = 0; round_id < profile_round; ++round_id) {
    delete[] operation_counts_profiles[round_id];
    operation_counts_profiles[round_id] = nullptr;
  }

  delete[] operation_counts_profiles;
  operation_counts_profiles = nullptr;

  delete[] operation_counts;
  operation_counts = nullptr;

  delete[] init_keys;
  init_keys = nullptr;
}

int main(int argc, char* argv[]) {

  Config config;

  parse_args(argc, argv, config);
  
  run_workload(config);
  
}
