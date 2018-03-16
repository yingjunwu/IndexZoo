#pragma once
#include <immintrin.h>

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
    volatile __m256 data_vec = _mm256_load_ps(data + offset);
    // volatile __m256 ret_vec = _mm256_cmp_ps(data_vec, value_vec, _CMP_EQ_OS);
    // _mm256_store_ps(ret + offset, ret_vec);
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
