#pragma once

#include "learned_index.h"
#include "stx_btree_index.h"

enum class IndexType {
  LearnedIndexType = 0,
  StxBtreeIndexType,
};
