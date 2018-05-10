#include "experiments.h"


template<typename KeyT>
void test(std::function<int(const KeyT*, const size_t, const KeyT&)> search_func, const std::string &name) {

  size_t size = 24;

  KeyT *data = new KeyT[size];

  prepare_data<KeyT>(data, size);

  for (size_t i = 0; i < size; ++i) {
    int pos = search_func(data, size, data[i]);
    if (pos != i) {
      std::cerr << "wrong pos: " << pos << " " << i << " " << name << " " << sizeof(KeyT) << std::endl;
    }
  }
}

template<typename KeyT>
void test() {
  // test<KeyT>(binary_search<KeyT>, "binary");
  // test<KeyT>(interpolation_search<KeyT>, "interpolation");
  // test<KeyT>(interpolation_scan_search<KeyT>, "interpolation_scan");
  // test<KeyT>(scan_search<KeyT>, "scan");
  test<KeyT>(scan_search_simd<KeyT>, "scan_simd");
} 

void test() {
  // test<uint8_t>();
  // test<uint16_t>();
  // test<uint32_t>();
  test<uint64_t>();
}


int main() {
  test();
}