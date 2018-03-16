#pragma once

#include <cassert>
#include <unistd.h>

#define ASSERT(condition, string) \
  if ((!(condition))) { \
    std::cerr << "ASSERT @L" << __LINE__ << "/" << __FILE__ << ": " << string << std::endl; \
    raise(SIGTRAP); \
  }

const size_t SIMD_REGISTER_SIZE = 256; // bit
const size_t MAX_SIMD_FLOAT_COUNT = SIMD_REGISTER_SIZE / sizeof(float) / 8;
const size_t MAX_SIMD_DOUBLE_COUNT = SIMD_REGISTER_SIZE / sizeof(double) / 8;
const size_t MAX_SIMD_INT32_COUNT = SIMD_REGISTER_SIZE / sizeof(int32_t) / 8;
const size_t MAX_SIMD_INT64_COUNT = SIMD_REGISTER_SIZE / sizeof(int64_t) / 8;

const long PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

inline bool is_aligned(const void* ptr, std::uintptr_t alignment) noexcept {
  auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
  return !(iptr % alignment);
}

template<typename T>
T* alloc_align(const size_t alignment, const size_t count) {
  T *ret;
  int rt = 0;
  rt = posix_memalign((void**)&ret, alignment, count * sizeof(T));
  assert(rt == 0);
  assert(is_aligned(ret, alignment) == true);
  return ret;
}