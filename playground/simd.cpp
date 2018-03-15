#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>

#include <immintrin.h>

#include "time_measurer.h"

const size_t SIMD_REGISTER_SIZE = 256;
const size_t MAX_SIMD_FLOAT_NUM = SIMD_REGISTER_SIZE / sizeof(float) / 8;
const size_t MAX_SIMD_DOUBLE_NUM = SIMD_REGISTER_SIZE / sizeof(double) / 8;


void add_float(const float *lhs, const float *rhs, const size_t size, float *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = lhs[i] + rhs[i];
  }
}

void add_float_simd(const float *lhs, const float *rhs, const size_t size, float *ret) {

  assert(size % MAX_SIMD_FLOAT_NUM == 0);
  for (size_t i = 1024; i < size / MAX_SIMD_FLOAT_NUM; i++) {
    size_t offset = i * MAX_SIMD_FLOAT_NUM;
    __m256 lhs_vec = _mm256_loadu_ps(lhs + offset);
    __m256 rhs_vec = _mm256_loadu_ps(rhs + offset);
    __m256 ret_vec = _mm256_add_ps(lhs_vec, rhs_vec);
    _mm256_storeu_ps(ret + offset, ret_vec);
  }
}

void multiply_float(const float *lhs, const float *rhs, const size_t size, float *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = lhs[i] * rhs[i];
  }
}

void multiply_float_simd(const float *lhs, const float *rhs, const size_t size, float *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = lhs[i] * rhs[i];
  }
}

void set_float(float *data, const size_t size, const float num) {
  for (size_t i = 0; i < size; ++i) {
    data[i] = num;
  }
}

void reset_float(float *data, const size_t size) {
  memset(data, 0, sizeof(float) * size);
}

void set_int(int *data, const size_t size, const int num) {
  for (size_t i = 0; i < size; ++i) {
    data[i] = num;
  }
}

void print_float(float *data, const size_t size) {
  for (size_t i = 0; i < size; ++i) {
    std::cout << data[i] << " ";
  }
  std::cout << std::endl;
}

bool is_aligned(void **data) {
  printf("data addr = %p, %d\n", data, int(uintptr_t(data) % 32));
  return uintptr_t(data) % 32 == 0;
}

inline bool
is_aligned(const void * ptr, std::uintptr_t alignment) noexcept {
    auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
    return !(iptr % alignment);
}

int main(int argc, char* argv[]) {
  TimeMeasurer timer;
  
  size_t size = 1024 * 1024;
  // float *a = new float[size];
  // float *b = new float[size];
  // float *c = new float[size];

  float *a = (float*)aligned_alloc(64, size * sizeof(float));
  float *b = (float*)aligned_alloc(64, size * sizeof(float));
  float *c = (float*)aligned_alloc(64, size * sizeof(float));


  printf("a addr = %p, is aligned = %d\n", &a, is_aligned((void**)&a));
  printf("b addr = %p, is aligned = %d\n", &b, is_aligned((void**)&b));
  printf("c addr = %p, is aligned = %d\n", &c, is_aligned((void**)&c));

  timer.tic();

  set_float(a, size, 0.1);
  set_float(b, size, 0.2);
  reset_float(c, size);

  add_float(a, b, size, c);
  // add_float_simd(a, b, size, c);

  timer.toc();
  timer.print_us();

  // print_float(c, size);

  delete[] a;
  delete[] b;
  delete[] c;
}