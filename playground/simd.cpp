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
void compare_eq(const T *data, const size_t size, const T value, bool *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = (data[i] == value);
  }
}

template<typename T>
void compare_lt(const T *data, const size_t size, const T value, bool *ret) {
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

  assert(size % MAX_SIMD_FLOAT_COUNT == 0);
  for (size_t i = 1024; i < size / MAX_SIMD_FLOAT_COUNT; i++) {
    size_t offset = i * MAX_SIMD_FLOAT_COUNT;
    __m256 lhs_vec = _mm256_load_ps(lhs + offset);
    __m256 rhs_vec = _mm256_load_ps(rhs + offset);
    __m256 ret_vec = _mm256_add_ps(lhs_vec, rhs_vec);
    _mm256_store_ps(ret + offset, ret_vec);
  }
}

// template<>
// void add_simd<int32_t>(const int32_t *lhs, const int32_t *rhs, const size_t size, int32_t *ret) {

//   assert(size % MAX_SIMD_FLOAT_COUNT == 0);
//   for (size_t i = 1024; i < size / MAX_SIMD_INT32_COUNT; i++) {
//     size_t offset = i * MAX_SIMD_INT32_COUNT;
//     __m256i lhs_vec = _mm256_load_si256(lhs + offset);
//     __m256i rhs_vec = _mm256_load_si256(rhs + offset);
//     __m256i ret_vec = _mm256_add_epi32(lhs_vec, rhs_vec);
//     _mm256_store_si256(ret + offset, ret_vec);
//   }
// }

template<typename T>
void multiply_simd(const T *lhs, const T *rhs, const size_t size, T *ret);
  
template<>
void multiply_simd<float>(const float *lhs, const float *rhs, const size_t size, float *ret) {

  assert(size % MAX_SIMD_FLOAT_COUNT == 0);
  for (size_t i = 1024; i < size / MAX_SIMD_FLOAT_COUNT; i++) {
    size_t offset = i * MAX_SIMD_FLOAT_COUNT;
    __m256 lhs_vec = _mm256_load_ps(lhs + offset);
    __m256 rhs_vec = _mm256_load_ps(rhs + offset);
    __m256 ret_vec = _mm256_mul_ps(lhs_vec, rhs_vec);
    _mm256_store_ps(ret + offset, ret_vec);
  }
}



template<typename T>
void compare_eq(const T *data, const size_t size, const T value, bool *ret);

template<>
void compare_eq<float>(const float *data, const size_t size, const float value, bool *ret) {

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

template<typename T>
T* alloc_align(const size_t alignment, const size_t count) {
  T *ret;
  int rt = 0;
  rt = posix_memalign((void**)&ret, alignment, count * sizeof(T));
  assert(rt == 0);
  return ret;
}

inline bool is_aligned(const void* ptr, std::uintptr_t alignment) noexcept {
  auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
  return !(iptr % alignment);
}

typedef float T;

TimeMeasurer timer;

void perform_add(const T *a, const T *b, const size_t count, T *c) {

  timer.tic();

  for (size_t i = 0; i < 10; ++i) {
    add<T>(a, b, count, c);
  }
  
  timer.toc();

  timer.print_us();
  timer.print_ms();

}

void perform_multiply(const T *a, const T *b, const size_t count, T *c) {
  multiply<T>(a, b, count, c);
  print<T>(c, count);
}

int main(int argc, char* argv[]) {
  
  // size_t count = 1024 * 1024 * 1024;
  size_t count = 8;

  long alignment = sysconf(_SC_PAGE_SIZE);
  std::cout << "alignment = " << alignment << std::endl;
  
  T *a = alloc_align<T>(alignment, count);
  T *b = alloc_align<T>(alignment, count);
  T *c = alloc_align<T>(alignment, count);

  printf("a addr = %p, is aligned = %d\n", a, is_aligned(a, alignment));
  printf("b addr = %p, is aligned = %d\n", b, is_aligned(b, alignment));
  printf("c addr = %p, is aligned = %d\n", c, is_aligned(c, alignment));

  set<T>(a, count, 0.1);
  set<T>(b, count, 0.2);
  set_zero<T>(c, count);

  uint64_t *int_a = alloc_align<uint64_t>(alignment, 8);
  uint64_t *int_b = alloc_align<uint64_t>(alignment, 8);
  uint64_t *int_c = alloc_align<uint64_t>(alignment, 8);
  
  set_seq<uint64_t>(int_a, 8); 
  set_seq<uint64_t>(int_b, 8);
  set_zero<uint64_t>(int_c, 8);

  __m256i lhs_vec = _mm256_load_si256((__m256i*)int_a);
  __m256i rhs_vec = _mm256_load_si256((__m256i*)int_b);
  __m256i ret_vec = _mm256_add_epi64(lhs_vec, rhs_vec);
  __m256i *my_data = (__m256i*)int_c;
  _mm256_store_si256(my_data, ret_vec);
  // for (size_t i = 0; i < 8; ++i) {
  //   std::cout << int_c[i] << std::endl;
  // }

  // perform_multiply(a, b, count, c);

  delete[] a;
  delete[] b;
  delete[] c;
}