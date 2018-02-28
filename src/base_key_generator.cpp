#include "base_key_generator.h"

std::atomic<uint64_t> BaseKeyGenerator::global_curr_key_(0);
uint64_t BaseKeyGenerator::global_max_key_ = 0;
