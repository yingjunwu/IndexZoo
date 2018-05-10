#include <iostream>
#include <algorithm>
#include <vector>
#include <cassert>
#include <functional>

#include "immintrin.h"

#include "time_measurer.h"
#include "fast_random.h"

const size_t SIMD_SIZE = 128 / 8;
const size_t MAX_SIMD_UINT64_COUNT = SIMD_SIZE / sizeof(uint64_t);
const size_t SIMD_UINT64_MASK = (1 << MAX_SIMD_UINT64_COUNT) - 1;

const size_t MAX_SIMD_UINT32_COUNT = SIMD_SIZE / sizeof(uint32_t);
const size_t SIMD_UINT32_MASK = (1 << MAX_SIMD_UINT32_COUNT) - 1;

const size_t MAX_SIMD_UINT16_COUNT = SIMD_SIZE / sizeof(uint16_t);
const size_t SIMD_UINT16_MASK = (1 << MAX_SIMD_UINT16_COUNT) - 1;

const size_t MAX_SIMD_UINT8_COUNT = SIMD_SIZE / sizeof(uint8_t);
const size_t SIMD_UINT8_MASK = (1 << MAX_SIMD_UINT8_COUNT) - 1;


template<typename T>
inline int binary_search_helper(const T *data, const size_t size, const T &key, const int begin_offset, const int end_offset) {
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
inline int binary_search(const T *data, const size_t size, const T &key) {
  return binary_search_helper<T>(data, size, key, 0, size - 1);
}

template<typename T>
inline int interpolation_search_helper(const T *data, const size_t size, const T &key, const int begin_offset, const int end_offset) {
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
inline int interpolation_search(const T *data, const size_t size, const T &key) { 
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
inline int scan_search(const T *data, const size_t size, const T &key) {
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
inline int scan_search_simd(const T *data, const size_t size, const T &key);

template<>
inline int scan_search_simd(const uint8_t *data, const size_t size, const uint8_t &key) {
  if (key < data[0] || key > data[size - 1]) {
    return -1;
  }

  size_t i = 0;
  for (; i + MAX_SIMD_UINT8_COUNT - 1 < size; ++i) {
    __m128i key_vec =_mm_set1_epi8(key);
    __m128i data_vec = _mm_loadu_si128((__m128i*)(data + i));
    __m128i xmm_mask = _mm_cmpgt_epi8(key_vec, data_vec);
    
    unsigned index = _mm_movemask_ps(_mm_castsi128_ps(xmm_mask));
    if ((index & SIMD_UINT8_MASK) == SIMD_UINT8_MASK) {
      // key is larger than the data in comparison
      continue;
    } else {
      if (i == 0) {
        for (size_t j = i; j < i + MAX_SIMD_UINT8_COUNT; ++j) {
          if (data[j] == key) {
            return j;
          }
        }
      } else {
        for (size_t j = i - 1; j < i + MAX_SIMD_UINT8_COUNT; ++j) {
          if (data[j] == key) {
            return j;
          }
        } 
      }
      return -1;
    }
  }

  for (; i < size; ++i) {
    if (data[i] == key) {
      return i;
    } 
    if (data[i] > key) {
      return -1;
    }
  }
  return -1;
}

template<>
inline int scan_search_simd(const uint16_t *data, const size_t size, const uint16_t &key) {
  if (key < data[0] || key > data[size - 1]) {
    return -1;
  }

  size_t i = 0;
  for (; i + MAX_SIMD_UINT16_COUNT - 1 < size; ++i) {
    __m128i key_vec =_mm_set1_epi16(key);
    __m128i data_vec = _mm_loadu_si128((__m128i*)(data + i));
    __m128i xmm_mask = _mm_cmpgt_epi16(key_vec, data_vec);
    
    unsigned index = _mm_movemask_ps(_mm_castsi128_ps(xmm_mask));
    if ((index & SIMD_UINT16_MASK) == SIMD_UINT16_MASK) {
      // key is larger than the data in comparison
      continue;
    } else {
      if (i == 0) {
        for (size_t j = i; j < i + MAX_SIMD_UINT16_COUNT; ++j) {
          if (data[j] == key) {
            return j;
          }
        }
      } else {
        for (size_t j = i - 1; j < i + MAX_SIMD_UINT16_COUNT; ++j) {
          if (data[j] == key) {
            return j;
          }
        } 
      }
      return -1;
    }
  }

  for (; i < size; ++i) {
    if (data[i] == key) {
      return i;
    } 
    if (data[i] > key) {
      return -1;
    }
  }
  return -1;
}


template<>
inline int scan_search_simd(const uint32_t *data, const size_t size, const uint32_t &key) {
  if (key < data[0] || key > data[size - 1]) {
    return -1;
  }

  size_t i = 0;
  for (; i + MAX_SIMD_UINT32_COUNT - 1 < size; ++i) {
    __m128i key_vec =_mm_set1_epi32(key);
    __m128i data_vec = _mm_loadu_si128((__m128i*)(data + i));
    __m128i xmm_mask = _mm_cmpgt_epi32(key_vec, data_vec);
    
    unsigned index = _mm_movemask_ps(_mm_castsi128_ps(xmm_mask));
    if ((index & SIMD_UINT32_MASK) == SIMD_UINT32_MASK) {
      // key is larger than the data in comparison
      continue;
    } else {
      if (i == 0) {
        for (size_t j = i; j < i + MAX_SIMD_UINT32_COUNT; ++j) {
          if (data[j] == key) {
            return j;
          }
        }
      } else {
        for (size_t j = i - 1; j < i + MAX_SIMD_UINT32_COUNT; ++j) {
          if (data[j] == key) {
            return j;
          }
        } 
      }
      return -1;
    }
  }

  for (; i < size; ++i) {
    if (data[i] == key) {
      return i;
    } 
    if (data[i] > key) {
      return -1;
    }
  }
  return -1;
}


template<>
inline int scan_search_simd(const uint64_t *data, const size_t size, const uint64_t &key) {
  if (key < data[0] || key > data[size - 1]) {
    return -1;
  }

  size_t i = 0;
  for (; i + MAX_SIMD_UINT64_COUNT - 1 < size; ++i) {
    __m128i key_vec =_mm_set1_epi64((__m64)key);
    __m128i data_vec = _mm_loadu_si128((__m128i*)(data + i));
    __m128i xmm_mask = _mm_cmpgt_epi64(key_vec, data_vec);
    
    unsigned index = _mm_movemask_ps(_mm_castsi128_ps(xmm_mask));
    if ((index & SIMD_UINT64_MASK) == SIMD_UINT64_MASK) {
      // key is larger than the data in comparison
      continue;
    } else {
      if (i == 0) {
        for (size_t j = i; j < i + MAX_SIMD_UINT64_COUNT; ++j) {
          if (data[j] == key) {
            return j;
          }
        }
      } else {
        for (size_t j = i - 1; j < i + MAX_SIMD_UINT64_COUNT; ++j) {
          if (data[j] == key) {
            return j;
          }
        } 
      }
      return -1;
    }
  }

  for (; i < size; ++i) {
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
inline bool compare_func(T &lhs, T &rhs) {
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
void measure_performance(std::function<int(const T*, const size_t, const T&)> search_func, const size_t size, const size_t loop) {
  FastRandom rand;
  
  T *data = new T[size];

  prepare_data<T>(data, size);
  // print_data<T>(data, size);

  TimeMeasurer timer;
  long long total_time = 0;

  total_time = 0;
  timer.tic();
  volatile int pos;
  for (size_t i = 0; i < loop; ++i) {
    T key = rand.next<T>() % size;
    pos = search_func(data, size, key);
    // std::cout << "pos = " << pos << std::endl;
  }
  timer.toc();
  std::cout << "elapsed time: " << timer.time_us() * 1.0 / loop << " us" << std::endl;

  delete[] data;
  data = nullptr;

}


enum class SearchType {
  Binary = 0,
  Interpolation,
  InterpolationScan,
  Scan,
  ScanSIMD,
};

template<typename KeyT>
void measure_performance(const SearchType search_type, const size_t size, const size_t loop) {
    if (search_type == SearchType::Binary) {
    measure_performance<KeyT>(binary_search<KeyT>, size, loop);
  } else if (search_type == SearchType::Interpolation) {
    measure_performance<KeyT>(interpolation_search<KeyT>, size, loop);
  } else if (search_type == SearchType::InterpolationScan) {
    measure_performance<KeyT>(interpolation_scan_search<KeyT>, size, loop);
  } else if (search_type == SearchType::Scan) {
    measure_performance<KeyT>(scan_search<KeyT>, size, loop);
  } else if (search_type == SearchType::ScanSIMD) {
    measure_performance<KeyT>(scan_search_simd<KeyT>, size, loop);
  } else {
    std::cerr << "incorrect search type!" << std::endl;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    std::cerr << "usage: " << argv[0] << " search_type key_size size loop" << std::endl;
    return -1;
  }
  SearchType search_type = (SearchType)atoi(argv[1]);
  size_t key_size = atoi(argv[2]);
  size_t size = atoi(argv[3]);
  size_t loop = atoi(argv[4]);

  if (key_size == 8) {
    measure_performance<uint8_t>(search_type, size, loop);
  } else if (key_size == 16) {
    measure_performance<uint16_t>(search_type, size, loop);
  } else if (key_size == 32) {
    measure_performance<uint32_t>(search_type, size, loop);
  } else if (key_size == 64) {
    measure_performance<uint64_t>(search_type, size, loop);
  } else {
    std::cerr << "incorrect key size!" << std::endl;
  }
}


