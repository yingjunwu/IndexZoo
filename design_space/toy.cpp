#include <iostream>

int main() {
  int64_t a = 100;
  int64_t b = 99;
  // for (size_t i = 0; i < 8; ++i) {
  //   std::cout << (int)(((char*)(&a))[i]) << std::endl;
  // }
  int64_t c = a<<32 | b;
  for (size_t i = 0; i < 8; ++i) {
    std::cout << (int)(((char*)(&c))[i]) << std::endl;
  }

  int64_t d = (c >> 32);
  int64_t e = c & 0xFFFFFFFF;
  std::cout << "d=" << d << std::endl;
  std::cout << "e=" << e << std::endl;
}