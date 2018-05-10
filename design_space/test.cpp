#include <iostream>

#include "fast_random.h"

int main() {
  FastRandom rand;
  for (size_t i = 0; i < 10; ++i) {
    std::cout << rand.next(8) << std::endl; 
  }
}