#include "experiments.h"

template<typename KeyT>
void measure(std::function<int(const KeyT*, const size_t, const KeyT&)> search_func, const size_t size, const size_t loop) {
  FastRandom rand;
  
  KeyT *data = new KeyT[size];

  prepare_data<KeyT>(data, size);
  // print_data<KeyT>(data, size);

  TimeMeasurer timer;
  long long total_time = 0;

  total_time = 0;
  timer.tic();
  volatile int pos;
  for (size_t i = 0; i < loop; ++i) {
    KeyT key = rand.next<KeyT>() % size;
    pos = search_func(data, size, key);
  }
  timer.toc();
  std::cout << "elapsed time: " << timer.time_us() * 1.0 / loop << " us" << std::endl;

  delete[] data;
  data = nullptr;

}

template<typename KeyT>
void measure(const SearchType search_type, const size_t size, const size_t loop) {
    if (search_type == SearchType::Binary) {
    measure<KeyT>(binary_search<KeyT>, size, loop);
  } else if (search_type == SearchType::Interpolation) {
    measure<KeyT>(interpolation_search<KeyT>, size, loop);
  } else if (search_type == SearchType::InterpolationScan) {
    measure<KeyT>(interpolation_scan_search<KeyT>, size, loop);
  } else if (search_type == SearchType::Scan) {
    measure<KeyT>(scan_search<KeyT>, size, loop);
  } else if (search_type == SearchType::ScanSIMD) {
    measure<KeyT>(scan_search_simd<KeyT>, size, loop);
  } else {
    std::cerr << "incorrect search type!" << std::endl;
  }
}

void measure(const SearchType search_type, const size_t key_size, const size_t size, const size_t loop) {
  if (key_size == 8) {
    measure<uint8_t>(search_type, size, loop);
  } else if (key_size == 16) {
    measure<uint16_t>(search_type, size, loop);
  } else if (key_size == 32) {
    measure<uint32_t>(search_type, size, loop);
  } else if (key_size == 64) {
    measure<uint64_t>(search_type, size, loop);
  } else {
    std::cerr << "incorrect key size!" << std::endl;
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

  measure(search_type, key_size, size, loop);
}


