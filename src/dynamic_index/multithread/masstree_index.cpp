#include "masstree_index.h"

volatile uint64_t globalepoch = 1;
volatile bool recovering = false;
thread_local threadinfo *ti_ = nullptr;
