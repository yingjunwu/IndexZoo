#include <iostream>

#include "fast_random.h"

int main() {
  FastRandom rand;
  for (size_t i = 0; i < 10; ++i) {
    std::cout << rand.next<uint8_t>() << std::endl; 
  }
}