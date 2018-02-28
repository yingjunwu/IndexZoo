#include "uint64_sequence_key_generator.h"

std::atomic<uint64_t> Uint64SequenceKeyGenerator::global_curr_key_(0);
