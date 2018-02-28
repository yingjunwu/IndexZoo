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

#include "uint64_uniform_key_generator.h"
#include "uint64_normal_key_generator.h"

#include "data_table.h"

#include "index_all.h"

void usage(FILE *out) {
  fprintf(out,
          "Command line options : olap_benchmark <options> \n"
          "   -h --help              :  print help message \n"
          "   -i --index             :  index type: \n"
          "                              -- interpolation_index (default) \n"
          "                              -- stx_btree \n"
          "   -y --read_type         :  read type: \n"
          "                              -- (0) index lookup (default) \n"
          "                              -- (1) index scan \n"
          "                              -- (2) index reverse scan \n"
          "   -t --time_duration     :  time duration (default: 10) \n"
          "   -m --key_count         :  key count (default: 1ull<<20) \n"
          "   -n --unique_key_count  :  unique key count (default: 0) \n"
          "   -r --reader_count      :  reader count (default: 1) \n"
          "   -d --distribution      :  data distribution: \n"
          "                              -- (0) uniform distribution (default) \n"
          "                              -- (1) normal distribution \n"
          "                              -- (2) log-normal distribution \n"
          "   -P --parameter_1       :  1st distribution parameter \n"
          "   -Q --parameter_2       :  2nd distribution parameter \n"
  );
}

static struct option opts[] = {
    { "index",             optional_argument, NULL, 'i' },
    { "read_type",         optional_argument, NULL, 'y' },
    { "time_duration",     optional_argument, NULL, 't' },
    { "key_count",         optional_argument, NULL, 'm' },
    { "unique_key_count",  optional_argument, NULL, 'n' },
    { "reader_count",      optional_argument, NULL, 'r' },
    { "distribution",      optional_argument, NULL, 'd' },
    { "parameter_1",       optional_argument, NULL, 'P' },
    { "parameter_2",       optional_argument, NULL, 'Q' },
    { NULL, 0, NULL, 0 }
};

enum class ReadType {
  IndexLookupType = 0,
  IndexScanType,
  IndexScanReverseType,
};

enum class DistributionType {
  UniformType = 0,
  NormalType,
  LogNormalType,
};

struct Config {
  IndexType index_type_ = IndexType::InterpolationIndexType;
  ReadType index_read_type_ = ReadType::IndexLookupType;
  DistributionType distribution_type_ = DistributionType::UniformType;
  double parameter_1_ = 0;
  double parameter_2_ = 0;
  uint64_t time_duration_ = 10;
  double profile_duration_ = 0.5;
  uint64_t key_count_ = 1ull << 20;
  // if unique_key_count_ is set to 0, then generate insert key sequentially.
  uint64_t unique_key_count_ = 0;
  uint64_t reader_count_ = 1;
};

void parse_args(int argc, char* argv[], Config &config) {
  
  while (1) {
    int idx = 0;
    int c = getopt_long(argc, argv, "ht:m:n:r:i:y:d:P:Q:", opts, &idx);

    if (c == -1) break;

    switch (c) {
      case 'i': {
        char *index = optarg;
        if (strcmp(index, "stx_btree") == 0) {
          config.index_type_ = IndexType::StxBtreeIndexType;
        } else if (strcmp(index, "interpolation_index") == 0) {
          config.index_type_ = IndexType::InterpolationIndexType;
        } else {
          fprintf(stderr, "Unknown index: %s\n", index);
          exit(EXIT_FAILURE);
        }
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
      case 'n': {
        config.unique_key_count_ = (uint64_t)atoi(optarg);
        break;
      }
      case 'r': {
        config.reader_count_ = (uint64_t)atoi(optarg);
        break;
      }
      case 'P': {
        config.parameter_1_ = (double)atof(optarg);
        break;
      }
      case 'Q': {
        config.parameter_2_ = (double)atof(optarg);
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

}

typedef Uint64 KeyT;
typedef Uint64 ValueT;


bool is_running = false;
uint64_t *operation_counts = nullptr;


// table and index
std::unique_ptr<DataTable<KeyT, ValueT>> data_table(nullptr);
std::unique_ptr<BaseIndex<KeyT>> data_index(nullptr);

void run_reader_thread(const uint64_t &thread_id, const Config &config) {

  pin_to_core(thread_id);

  std::unique_ptr<BaseKeyGenerator> key_generator;
  if (config.distribution_type_ == DistributionType::UniformType) {

    key_generator.reset(new Uint64UniformKeyGenerator(thread_id));

  } else if (config.distribution_type_ == DistributionType::NormalType) {

    key_generator.reset(new Uint64NormalKeyGenerator(thread_id, config.parameter_1_, config.parameter_2_));
  
  } else {
    assert(config.distribution_type_ == DistributionType::LogNormalType);

    key_generator.reset(new Uint64NormalKeyGenerator(thread_id, config.parameter_1_, config.parameter_2_));
  
  }

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
    assert(config.index_read_type_ == ReadType::IndexScanReverseType);

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
  
  std::unique_ptr<BaseKeyGenerator> key_generator;
  if (config.distribution_type_ == DistributionType::UniformType) {

    key_generator.reset(new Uint64UniformKeyGenerator(0));

  } else if (config.distribution_type_ == DistributionType::NormalType) {

    key_generator.reset(new Uint64NormalKeyGenerator(0, config.parameter_1_, config.parameter_2_));
  
  } else {
    assert(config.distribution_type_ == DistributionType::LogNormalType);

    key_generator.reset(new Uint64NormalKeyGenerator(0, config.parameter_1_, config.parameter_2_));
  
  
  }

  for (size_t i = 0; i < config.key_count_; ++i) {

    KeyT key = key_generator->get_insert_key();
    ValueT value = 100;
    
    OffsetT offset = data_table->insert_tuple(key, value);

    data_index->insert(key, offset.raw_data());
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

  double init_mem_size = get_memory_gb();
  std::cout << "init memory size = " << init_mem_size << " GB" << std::endl;
  
  // launch a group of threads
  is_running = true;
  std::vector<std::thread> worker_threads;
  uint64_t thread_count = 0;

  // reader threads
  for (; thread_count < config.reader_count_; ++thread_count) {
    worker_threads.push_back(std::move(std::thread(run_reader_thread, thread_count, config)));
  }

  std::cout << "        TIME         READ       RAM (act.)   RAM (est.)" << std::endl;

  for (uint64_t round_id = 0; round_id < profile_round; ++round_id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(int(config.profile_duration_ * 1000)));
    
    memcpy(operation_counts_profiles[round_id], operation_counts, sizeof(uint64_t) * config.reader_count_);

    act_size_profiles.push_back(get_memory_gb());
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
              << " GB  |  "
              << std::setw(5)
              << approx_size_profiles.at(round_id) * (sizeof(KeyT) + sizeof(ValueT)) * 1.0 / 1024 / 1024 / 1024
              << " GB"
              << std::endl;
  }
  
  // join all the threads
  is_running = false;

  for (uint64_t i = 0; i < config.reader_count_; ++i) {
    worker_threads.at(i).join();
  }

  std::string index_name;
  if (config.index_type_ == IndexType::StxBtreeIndexType) {
    index_name = "stx_btree";
  } else if (config.index_type_ == IndexType::InterpolationIndexType) {
    index_name = "interpolation_index";
  }
  
  uint64_t total_count = 0;
  for (uint64_t i = 0; i < config.reader_count_; ++i) {
    total_count += operation_counts[i];
  }

  std::cout << "index = " << index_name.c_str() << ", "
            << "read = " << config.reader_count_ << ", "
            << "throughput = " << total_count * 1.0 / config.time_duration_ / 1000 / 1000 << " M ops" 
            << std::endl;

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

  BaseKeyGenerator::set_max_key(config.unique_key_count_);

  data_table.reset(new DataTable<KeyT, ValueT>());
  
  if (config.index_type_ == IndexType::StxBtreeIndexType) {

    data_index.reset(new StxBtreeIndex<KeyT>());

  } else if (config.index_type_ == IndexType::InterpolationIndexType) {

    data_index.reset(new InterpolationIndex<KeyT>());
  
  } else {
    assert(false);
  }

  run_workload(config);
  
}
