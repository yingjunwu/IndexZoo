#include <cstdlib>
#include <immintrin.h>

int main() {
  size_t size = 10;
  float *lhs = new float[size];
  float *rhs = new float[size];
  float *ret = new float[size];
  for (size_t i = 0; i < size; i++) {
    // ret[i] = lhs[i] + rhs[i];
    __m256 lhs_vec = _mm256_load_ps(lhs);
    __m256 rhs_vec = _mm256_load_ps(rhs);
    __m256 ret_vec = _mm256_add_ps(lhs_vec, rhs_vec);
  }

}