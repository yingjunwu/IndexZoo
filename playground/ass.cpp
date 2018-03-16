#include <cstdlib>

int main() {
  size_t size = 10;
  float *lhs = new float[size];
  float *rhs = new float[size];
  float *ret = new float[size];
  for (size_t i = 0; i < size; i++) {
    ret[i] = lhs[i] + rhs[i];
  }

}