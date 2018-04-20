#include <iostream>
#include <algorithm>
#include <vector>
#include <cassert>
#include <functional>

#include "time_measurer.h"
#include "fast_random.h"

template<typename T>
int binary_search_helper(const T *data, const size_t size, const T &key, const int begin_offset, const int end_offset) {
  if (begin_offset > end_offset) {
    return -1;
  }
  if (key < data[begin_offset] || key > data[end_offset]) {
    return -1;
  }
  int guess = (begin_offset + end_offset) / 2;
  if (data[guess] == key) {
    return guess;
  }
  if (data[guess] > key) {
    return binary_search_helper<T>(data, size, key, begin_offset, guess - 1);
  } else {
    return binary_search_helper<T>(data, size, key, guess + 1, end_offset);
  }
}

template<typename T>
int binary_search(const T *data, const size_t size, const T &key) {
  return binary_search_helper<T>(data, size, key, 0, size - 1);
}

template<typename T>
int interpolation_search_helper(const T *data, const size_t size, const T &key, const int begin_offset, const int end_offset) {
  if (begin_offset > end_offset) {
    return -1;
  }
  if (key < data[begin_offset] || key > data[end_offset]) {
    return -1;
  }
  if (begin_offset == end_offset) {
    if (data[begin_offset] == key) {
      return begin_offset;
    } else {
      return -1;
    }
  }
  int guess = (key - data[begin_offset]) * 1.0 / (data[end_offset] - data[begin_offset]) * (end_offset - begin_offset) + begin_offset;
  if (data[guess] == key) {
    return guess;
  }
  if (data[guess] > key) {
    return interpolation_search_helper<T>(data, size, key, begin_offset, guess - 1);
  } else {
    return interpolation_search_helper<T>(data, size, key, guess + 1, end_offset);
  }
}

template<typename T>
int interpolation_search(const T *data, const size_t size, const T &key) { 
  return interpolation_search_helper<T>(data, size, key, 0, size - 1);
}

template<typename T>
inline int interpolation_scan_search(const T *data, const size_t size, const T &key) {
  if (data[size - 1] == data[0]) {
    if (data[0] == key) {
      return 0;
    } else {
      return -1;
    }
  }
  if (key < data[0] || key > data[size - 1]) {
    return -1;
  }
  int guess = (key - data[0]) * 1.0 / (data[size - 1] - data[0]) * (size - 1);
  if (data[guess] == key) {
    return guess;
  }
  if (data[guess] > key) {
    --guess;
    // move left
    while (guess >= 0) {
      if (data[guess] < key) {
        return -1;
      }
      if (data[guess] == key) {
        return guess;
      }
      --guess;
    };
    return -1;

  } else {
    ++guess;
    // move right
    while (guess < size) {
      if (data[guess] > key) {
        return -1;
      }
      if (data[guess] == key) {
        return guess;
      }
      ++guess;
    };
    return -1;
  }
}

template<typename T>
int scan_search(const T *data, const size_t size, const T &key) {
  if (key < data[0] || key > data[size - 1]) {
    return -1;
  }
  for (size_t i = 0; i < size; ++i) {
    if (data[i] == key) {
      return i;
    } 
    if (data[i] > key) {
      return -1;
    }
  }
  return -1;
}


template<typename T>
bool compare_func(T &lhs, T &rhs) {
  return lhs < rhs;
}

template<typename T>
void prepare_data(T *data, const size_t size) {
  FastRandom rand;
  for (size_t i = 0; i < size; ++i) {
    data[i] = rand.next<T>() % size;
  }
  std::sort(data, data + size, compare_func<T>);
}

template<typename T>
void print_data(T *data, const size_t size) {
  for (size_t i = 0; i < size; ++i) {
    std::cout << data[i] << " ";
  }
  std::cout << std::endl;
}

template<typename T>
void measure_performance(std::function<int(const T*, const size_t, const T&)> search_func, const size_t size) {
  FastRandom rand;
  
  T *data = new T[size];

  prepare_data<T>(data, size);
  // print_data<T>(data, size);

  TimeMeasurer timer;
  size_t loop = 1000;
  long long total_time = 0;

  total_time = 0;
  timer.tic();
  volatile int pos;
  for (size_t i = 0; i < loop; ++i) {
    T key = rand.next<T>() % size;
    pos = search_func(data, size, key);
  }
  timer.toc();
  timer.print_us();

  delete[] data;
  data = nullptr;

}

typedef uint64_t KeyT;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    return -1;
  }
  int method = atoi(argv[1]);
  size_t size = atoi(argv[2]);
  if (method == 0) {
    measure_performance<KeyT>(binary_search<KeyT>, size);
  } else if (method == 1) {
    measure_performance<KeyT>(interpolation_search<KeyT>, size);
  } else if (method == 2) {
    measure_performance<KeyT>(interpolation_scan_search<KeyT>, size);
  } else {
    measure_performance<KeyT>(scan_search<KeyT>, size);
  }
}