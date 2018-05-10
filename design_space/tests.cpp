#include "experiments.h"


template<typename KeyT>
void test(std::function<int(const KeyT*, const size_t, const KeyT&)> search_func) {

}

template<typename KeyT>
void test() {
  test<KeyT>(binary_search<KeyT>);
  test<KeyT>(interpolation_search<KeyT>);
  test<KeyT>(interpolation_scan_search<KeyT>);
  test<KeyT>(scan_search<KeyT>);
  test<KeyT>(scan_search_simd<KeyT>);
} 

void test() {
  test<uint8_t>();
  test<uint16_t>();
  test<uint32_t>();
  test<uint64_t>();
}


int main() {
  test();
}