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


struct Config {

};


void parse_args(int argc, char* argv[], Config &config) {}


template<typename ValueT>
void run_workload(const Config &config) {

}


int main(int argc, char* argv[]) {

  Config config;

  parse_args(argc, argv, config);

  run_workload<Uint64>(config);
  

}
