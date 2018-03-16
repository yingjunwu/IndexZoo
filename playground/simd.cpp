#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>

#include <immintrin.h>
#include <unistd.h>
#include "time_measurer.h"

const size_t SIMD_REGISTER_SIZE = 256;
const size_t MAX_SIMD_FLOAT_COUNT = SIMD_REGISTER_SIZE / sizeof(float) / 8;
const size_t MAX_SIMD_DOUBLE_COUNT = SIMD_REGISTER_SIZE / sizeof(double) / 8;
const size_t MAX_SIMD_INT32_COUNT = SIMD_REGISTER_SIZE / sizeof(int32_t) / 8;
const size_t MAX_SIMD_INT64_COUNT = SIMD_REGISTER_SIZE / sizeof(int64_t) / 8;

const long PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

template<typename T>
void add(const T *lhs, const T *rhs, const size_t size, T *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = lhs[i] + rhs[i];
  }
}

template<typename T>
void multiply(const T *lhs, const T *rhs, const size_t size, T *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = lhs[i] * rhs[i];
  }
}

template<typename T>
void compare_eq(const T *data, const size_t size, const T value, float *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = (data[i] == value);
  }
}

template<typename T>
void compare_lt(const T *data, const size_t size, const T value, float *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = (data[i] < value);
  }
}

template<typename T>
void compare_le(const T *data, const size_t size, const T value, bool *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = (data[i] <= value);
  }
}

template<typename T>
void compare_gt(const T *data, const size_t size, const T value, bool *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = (data[i] > value);
  }
}

template<typename T>
void compare_ge(const T *data, const size_t size, const T value, bool *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = (data[i] >= value);
  }
}

template<typename T>
void add_simd(const T *lhs, const T *rhs, const size_t size, T *ret);

template<>
void add_simd<float>(const float *lhs, const float *rhs, const size_t size, float *ret) {

  size_t offset = 0;
  for (size_t i = 0; i < size / MAX_SIMD_FLOAT_COUNT; i++) {
    __m256 lhs_vec = _mm256_load_ps(lhs + offset);
    __m256 rhs_vec = _mm256_load_ps(rhs + offset);
    __m256 ret_vec = _mm256_add_ps(lhs_vec, rhs_vec);
    _mm256_store_ps(ret + offset, ret_vec);
    offset += MAX_SIMD_FLOAT_COUNT;
  }
  if (size % MAX_SIMD_FLOAT_COUNT != 0) {
    add<float>(lhs + offset, rhs + offset, size % MAX_SIMD_FLOAT_COUNT, ret + offset);
  }
  
}

template<>
void add_simd<int32_t>(const int32_t *lhs, const int32_t *rhs, const size_t size, int32_t *ret) {

  size_t offset = 0;
  for (size_t i = 0; i < size / MAX_SIMD_INT32_COUNT; i++) {
    __m256i lhs_vec = _mm256_load_si256((__m256i*)(lhs + offset));
    __m256i rhs_vec = _mm256_load_si256((__m256i*)(rhs + offset));
    __m256i ret_vec = _mm256_add_epi32(lhs_vec, rhs_vec);
    _mm256_store_si256((__m256i*)(ret + offset), ret_vec);
    offset += MAX_SIMD_INT32_COUNT;
  }
  if (size % MAX_SIMD_INT32_COUNT != 0) {
    add<int32_t>(lhs + offset, rhs + offset, size % MAX_SIMD_INT32_COUNT, ret + offset);
  }
}

template<>
void add_simd<int64_t>(const int64_t *lhs, const int64_t *rhs, const size_t size, int64_t *ret) {

  size_t offset = 0;
  for (size_t i = 0; i < size / MAX_SIMD_INT64_COUNT; i++) {
    __m256i lhs_vec = _mm256_load_si256((__m256i*)(lhs + offset));
    __m256i rhs_vec = _mm256_load_si256((__m256i*)(rhs + offset));
    __m256i ret_vec = _mm256_add_epi64(lhs_vec, rhs_vec);
    _mm256_store_si256((__m256i*)(ret + offset), ret_vec);
    offset += MAX_SIMD_INT64_COUNT;
  }
  if (size % MAX_SIMD_INT64_COUNT != 0) {
    add<int64_t>(lhs + offset, rhs + offset, size % MAX_SIMD_INT64_COUNT, ret + offset);
  }
}

template<typename T>
void multiply_simd(const T *lhs, const T *rhs, const size_t size, T *ret);
  
template<>
void multiply_simd<float>(const float *lhs, const float *rhs, const size_t size, float *ret) {

    size_t offset = 0;
  for (size_t i = 0; i < size / MAX_SIMD_FLOAT_COUNT; i++) {
    __m256 lhs_vec = _mm256_load_ps(lhs + offset);
    __m256 rhs_vec = _mm256_load_ps(rhs + offset);
    __m256 ret_vec = _mm256_mul_ps(lhs_vec, rhs_vec);
    _mm256_store_ps(ret + offset, ret_vec);
    offset += MAX_SIMD_FLOAT_COUNT;
  }
  if (size % MAX_SIMD_FLOAT_COUNT != 0) {
    multiply<float>(lhs + offset, rhs + offset, size % MAX_SIMD_FLOAT_COUNT, ret + offset);
  }
  
}

template<typename T>
void compare_eq_simd(const T *data, const size_t size, const T value, float *ret);

template<>
void compare_eq_simd<float>(const float *data, const size_t size, const float value, float *ret) {
  
  __m256 value_vec = _mm256_set1_ps(value);

  size_t offset = 0;
  for (size_t i = 0; i < size / MAX_SIMD_FLOAT_COUNT; ++i) {
    __m256 data_vec = _mm256_load_ps(data + offset);
    __m256 ret_vec = _mm256_cmp_ps(data_vec, value_vec, _CMP_EQ_OS);
    _mm256_store_ps(ret + offset, ret_vec);
    offset += MAX_SIMD_FLOAT_COUNT;
  }
  // if (size % MAX_SIMD_FLOAT_COUNT != 0) {
  //   compare_eq<float>(data + offset, size % MAX_SIMD_FLOAT_COUNT, ret + offset);
  // }
}

template<typename T>
void compare_lt(const T *data, const size_t size, const T value, bool *ret);

template<>
void compare_lt<float>(const float *data, const size_t size, const float value, bool *ret) {

}

template<typename T>
void compare_le(const T *data, const size_t size, const T value, bool *ret);

template<>
void compare_le<float>(const float *data, const size_t size, const float value, bool *ret) {

}

template<typename T>
void compare_gt(const T *data, const size_t size, const T value, bool *ret);

template<>
void compare_gt<float>(const float *data, const size_t size, const float value, bool *ret) {

}

template<typename T>
void compare_ge(const T *data, const size_t size, const T value, bool *ret);

template<>
void compare_ge<float>(const float *data, const size_t size, const float value, bool *ret) {

}


template<typename T>
void set(T *data, const size_t size, const T value) {
  for (size_t i = 0; i < size; ++i) {
    data[i] = value;
  }
}

template<typename T>
void set_zero(T *data, const size_t size) {
  memset(data, 0, sizeof(T) * size);
}

template<typename T>
void set_seq(T *data, const size_t size) {
  for (size_t i = 0; i < size; ++i) {
    data[i] = i;
  }
}

template<typename T>
void print(T *data, const size_t size) {
  for (size_t i = 0; i < size; ++i) {
    std::cout << data[i] << " ";
  }
  std::cout << std::endl;
}

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


template<typename T>
void perform_add(const size_t count) {

  long alignment = PAGE_SIZE;
  
  T *a = alloc_align<T>(alignment, count);
  T *b = alloc_align<T>(alignment, count);
  T *c = alloc_align<T>(alignment, count);

  printf("addr = %p, %p, %p\n", a, b, c);
  
  set_seq<T>(a, count);
  set_seq<T>(b, count);
  set_zero<T>(c, count);

  TimeMeasurer timer;
  timer.tic();

  for (size_t i = 0; i < 10; ++i) {
    add_simd<T>(a, b, count, c);

  }
  
  timer.toc();

  timer.print_us();
  timer.print_ms();

  print(c, count);

  delete[] a;
  delete[] b;
  delete[] c;
}


template<typename T>
void perform_compare(const size_t count) {

  long alignment = PAGE_SIZE;
  
  T *a = alloc_align<T>(alignment, count);
  T *b = alloc_align<T>(alignment, count);

  printf("addr = %p, %p\n", a, b);
  
  set_seq<T>(a, count);
  set_zero<T>(b, count);

  TimeMeasurer timer;
  timer.tic();

  for (size_t i = 0; i < 10; ++i) {
    // compare_eq_simd<T>(a, count, 5, b);
    compare_eq<T>(a, count, 5, b);
  }
  
  timer.toc();

  timer.print_us();
  timer.print_ms();

  // print(b, count);

  delete[] a;
  delete[] b;
}

template<typename T>
void perform_multiply(const T *a, const T *b, const size_t count, T *c) {
  multiply<T>(a, b, count, c);
  print<T>(c, count);
}

int main(int argc, char* argv[]) {
  
  size_t count = 1024 * 1024 * 1024;
  // size_t count = 11;
  // perform_add<float>(count);
  
  perform_compare<float>(count);
}
