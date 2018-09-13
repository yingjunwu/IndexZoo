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
          "Command line options : index_benchmark <options> \n"
          "   -h --help              :  print help message \n"
          // index structure
          "   -i --index             :  index type: \n"
          "                              --  (0) static  - interpolation index (default) \n"
          "                              --  (1) static  - binary index \n"
          "                              --  (2) static  - kary index \n"
          "                              --  (3) static  - fast index \n"
          "                              -- (10) dynamic - singlethread - stx-btree index \n"
          "                              -- (11) dynamic - singlethread - art-tree index \n"
          "                              -- (12) dynamic - singlethread - skiplist index (unsupported) \n"
          "                              -- (13) dynamic - singlethread - btree index (unsupported) \n"
          "                              -- (20) dynamic - multithread  - libcuckoo index \n"
          "                              -- (21) dynamic - multithread  - art-tree index \n"
          "                              -- (22) dynamic - multithread  - bw-tree index \n"
          "                              -- (23) dynamic - multithread  - masstree index \n"
          "   -k --key_size          :  index key size (default: 8 bytes) \n"
          "   -S --index_param_1     :  1st index parameter \n"
          "   -T --index_param_2     :  2nd index parameter \n"
          // configuration
          "   -t --time_duration     :  time duration (default: 10) \n"
          "   -y --read_type         :  read type: \n"
          "                              -- (0) index lookup (default) \n"
          "                              -- (1) index scan \n"
          "                              -- (2) index reverse scan \n"
          "   -r --read_ratio        :  read ratio (default: 1.0) \n"
          "   -s --thread_count      :  thread count (default: 1) \n"
          "   -m --key_count         :  key count (default: 1ull<<20) \n"
          // numeric data distribution
          "   -d --distribution      :  numerical data distribution: \n"
          "                              -- (0) sequence (default) \n"
          "                              -- (1) uniform distribution \n"
          "                              -- (2) normal distribution \n"
          "                              -- (3) log-normal distribution \n"
          "   -P --key_bound         :  key upper bound \n"
          "   -Q --key_stddev        :  key standard deviation \n"
          // workload configuration
          // "   skewness \n"
          // "   for read, percentage of failed lookup \n"
          "   -c --record           :  record all keys \n"
          "   -v --verbose          :  verbose \n"
  );
}

static struct option opts[] = {
    // index structure
    { "index",             optional_argument, NULL, 'i' },
    { "key_size",          optional_argument, NULL, 'k' },
    { "index_param_1",     optional_argument, NULL, 'S' },
    { "index_param_2",     optional_argument, NULL, 'T' },
    // configuration
    { "time_duration",     optional_argument, NULL, 't' },
    { "read_type",         optional_argument, NULL, 'y' },
    { "read_ratio",        optional_argument, NULL, 'r' },
    { "thread_count",      optional_argument, NULL, 's' },
    // data distribution
    { "key_count",         optional_argument, NULL, 'm' },
    { "distribution",      optional_argument, NULL, 'd' },
    { "key_bound",         optional_argument, NULL, 'P' },
    { "key_stddev",        optional_argument, NULL, 'Q' },
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
  IndexType index_type_ = IndexType::S_Interpolation;
  int key_size_ = 8; // unit: bytes
  int index_param_1_ = INVALID_INDEX_PARAM;
  int index_param_2_ = INVALID_INDEX_PARAM;
  // configuration
  const double profile_duration_ = 0.5; // fixed
  int time_duration_ = 10;
  ReadType index_read_type_ = ReadType::IndexLookupType;
  double read_ratio_ = 1.0;
  int thread_count_ = 1;
  // data distribution
  uint64_t key_count_ = 1ull << 20;
  DistributionType distribution_type_ = DistributionType::SequenceType;
  uint64_t key_bound_ = DEFAULT_KEY_BOUND;
  double key_stddev_ = INVALID_KEY_STDDEV;
  bool record_ = false;
  bool verbose_ = false;
  uint64_t generated_read_key_count_ = 100 * 1000 * 1000; // 100 millions

  void print() {
    std::cout << "=====     INDEX STRUCTURE    =====" << std::endl;
    std::cout << "key size: " << key_size_ << std::endl;
    std::cout << "index param " << index_param_1_ << ", " << index_param_2_ << std::endl;
    std::cout << "===== WORKLOAD CONFIGURATION =====" << std::endl;
    std::cout << "read ratio: " << read_ratio_ << std::endl;
    std::cout << "thread count: " << thread_count_ << std::endl;
    std::cout << "=====    DATA DISTRIBUTION   =====" << std::endl;
    std::cout << "key count: " << key_count_ << std::endl;
    std::cout << "key bound: " << key_bound_ << std::endl;
    std::cout << "key stddev: " << key_stddev_ << std::endl;
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>" << std::endl;
  }
};

void validate_key_generator_params(const Config &config);

void parse_args(int argc, char* argv[], Config &config) {
  
  while (1) {
    int idx = 0;
    int c = getopt_long(argc, argv, "hcvi:k:S:T:t:y:r:s:m:d:P:Q:", opts, &idx);

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
      case 'S': {
        config.index_param_1_ = atoi(optarg);
        break;
      }
      case 'T': {
        config.index_param_2_ = atoi(optarg);
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
      case 'd': {
        config.distribution_type_ = (DistributionType)atoi(optarg);
        break;
      }
      case 'P': {
        config.key_bound_ = (uint64_t)strtoull(optarg, nullptr, 10); // uint64_t
        break;
      }
      case 'Q': {
        config.key_stddev_ = (double)atof(optarg);
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

  validate_index_params(config.index_type_, config.index_param_1_, config.index_param_2_);

  validate_key_generator_params(config.distribution_type_, config.key_bound_, config.key_stddev_);

  config.generated_read_key_count_ = config.generated_read_key_count_ * config.read_ratio_;
  
  config.print();

}

bool is_running = false;
uint64_t *operation_counts = nullptr;

template<typename KeyT, typename ValueT>
void run_thread(const size_t &thread_id, const Config &config, const KeyT *read_keys, DataTable<KeyT, ValueT> *data_table, BaseIndex<KeyT, ValueT> *data_index) {

  pin_to_core(thread_id);

  data_index->register_thread(thread_id);

  std::unique_ptr<BaseKeyGenerator<KeyT>> key_generator(construct_key_generator<KeyT>(config.distribution_type_, thread_id, config.key_bound_, config.key_stddev_));

  uint64_t &operation_count = operation_counts[thread_id];
  operation_count = 0;

  FastRandom rand_gen(thread_id);

  while (true) {
    if (is_running == false) {
      break;
    }

    double next_rand = rand_gen.next_uniform();

    if (next_rand < config.read_ratio_) {
      KeyT key = read_keys[operation_count % config.generated_read_key_count_];

      std::vector<Uint64> offsets;

      // retrieve tuple locations
      data_index->find(key, offsets);

      // ASSERT(offsets.size() == 1, "must be 1! " << key);
    } else {
      // insert
      KeyT key = key_generator->get_next_key();

      ValueT value = 100;
      
      OffsetT offset = data_table->insert_tuple(key, value);

      // insert tuple locations into index
      data_index->insert(key, offset.raw_data());
    }

    ++operation_count;
  }
}

template<typename KeyT, typename ValueT>
void run_workload(const Config &config) {

  // create table
  std::unique_ptr<DataTable<KeyT, ValueT>> data_table(nullptr);
  data_table.reset(new DataTable<KeyT, ValueT>());

  // create index
  std::unique_ptr<BaseIndex<KeyT, ValueT>> data_index(nullptr);
  data_index.reset(create_numeric_index<KeyT, ValueT>(config.index_type_, data_table.get(), config.index_param_1_, config.index_param_2_));

  // prepare threads
  data_index->prepare_threads(config.thread_count_);
  data_index->register_thread(0);

  //=================================
  // populate table
  //=================================
  std::unique_ptr<BaseKeyGenerator<KeyT>> key_generator(construct_key_generator<KeyT>(config.distribution_type_, 0, config.key_bound_, config.key_stddev_));

  KeyT *init_keys = new KeyT[config.key_count_]; // store all init keys

  for (size_t i = 0; i < config.key_count_; ++i) {

    KeyT key = key_generator->get_next_key();
    ValueT value = 100;
    
    OffsetT offset = data_table->insert_tuple(key, value);

    data_index->insert(key, offset.raw_data());

    // record init input keys
    init_keys[i] = key;
  }
  data_index->reorganize();
  //=================================

  //=================================
  // write all init keys to output file
  //=================================
  if (config.record_ == true) {

    std::ofstream record_file;
    record_file.open("data.txt");
    
    for (size_t i = 0; i < config.key_count_; ++i) {
      record_file << init_keys[i] << std::endl;
    }
    record_file.close();

    // return;
  }
  //=================================

  //=================================
  // prepare query keys
  //=================================
  KeyT** read_keys = new KeyT*[config.thread_count_];
  
  // generate keys for each thread
  for (size_t i = 0; i < config.thread_count_; ++i) {

    read_keys[i] = new KeyT[config.generated_read_key_count_];

    FastRandom rand_gen(i);
    
    for (size_t j = 0; j < config.generated_read_key_count_; ++j) {
      read_keys[i][j] = init_keys[rand_gen.next<uint64_t>() % config.key_count_];
    }
  }

  double query_key_size_mb = (config.key_count_ + config.generated_read_key_count_) * sizeof(KeyT) / 1024 / 1024;

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
    worker_threads.push_back(std::move(std::thread(run_thread<KeyT, ValueT>, thread_id, std::ref(config), read_keys[thread_id], data_table.get(), data_index.get())));
  }

  std::cout << "        TIME       THROUGHPUT   RAM (tot.)   RAM (tab.)" << std::endl;

  for (uint64_t round_id = 0; round_id < profile_round; ++round_id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(int(config.profile_duration_ * 1000)));
    
    memcpy(operation_counts_profiles[round_id], operation_counts, sizeof(uint64_t) * config.thread_count_);

    double table_size_approx = data_table->size_approx() * (sizeof(KeyT) + sizeof(ValueT)) * 1.0 / 1024 / 1024;

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
  
  if (config.key_size_ == 4) {
    run_workload<Uint32, Uint64>(config);
  }
  else if (config.key_size_ == 8) {
    run_workload<Uint64, Uint64>(config);
  } else {
    std::cerr << "do not support key size = " << config.key_size_ << std::endl;

  }
  
}
