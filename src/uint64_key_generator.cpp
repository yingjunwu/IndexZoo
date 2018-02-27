#include "uint64_key_generator.h"


std::atomic<uint64_t> Uint64KeyGenerator::global_curr_key_(0);
uint64_t Uint64KeyGenerator::global_max_key_ = 0;