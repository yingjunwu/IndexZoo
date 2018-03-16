#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>

#include "time_measurer.h"
#include "fast_random.h"
#include "utils.h"

#include "operations.h"
#include "operations_simd.h"

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
  // bool *b =alloc_align<bool>(alignment, count);

  printf("addr = %p, %p\n", a, b);
  
  set_seq<T>(a, count);
  set_zero<T>(b, count);
  // set_zero<bool>(b, count);

  TimeMeasurer timer;
  timer.tic();

  for (size_t i = 0; i < 10; ++i) {
    compare_eq_simd<T>(a, count, 5, b);
    // compare_eq<T>(a, count, 5, b);
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
  // perform_add<int32_t>(count);
  
  perform_compare<float>(count);
}
