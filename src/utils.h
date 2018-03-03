#pragma once

#include <jemalloc/jemalloc.h>
#include <cstdint>
#include <csignal>
#include <iostream>

typedef uint64_t Uint64;

#define COMPILER_MEMORY_FENCE asm volatile("" ::: "memory")


static double get_memory_mb() {
  uint64_t epoch = 1;
  size_t sz = sizeof(epoch);
  mallctl("epoch", &epoch, &sz, &epoch, sz);

  size_t allocated;
  sz = sizeof(size_t);
  if (mallctl("stats.allocated", &allocated, &sz, NULL, 0) == 0) {
    return allocated * 1.0 / 1024 / 1024;
  }
  return -1;
}

static double get_memory_gb() {
  uint64_t epoch = 1;
  size_t sz = sizeof(epoch);
  mallctl("epoch", &epoch, &sz, &epoch, sz);

  size_t allocated;
  sz = sizeof(size_t);
  if (mallctl("stats.allocated", &allocated, &sz, NULL, 0) == 0) {
    return allocated * 1.0 / 1024 / 1024 / 1024;
  }
  return -1;
}

static void pin_to_core(const size_t core) {
  #if 0
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  int ret = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
  if (ret != 0) {
    assert(false);
  }
  #endif
}

#define ASSERT(condition, string) \
  if ((!(condition))) { \
    std::cerr << "ASSERT @L" << __LINE__ << "/" << __FILE__ << ": " << string << std::endl; \
    raise(SIGTRAP); \
  }
  