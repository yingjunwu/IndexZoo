#pragma once

#include "interpolation_index.h"
#include "stx_btree_index.h"
#include "interpolation_index_v1.h"

enum class IndexType {
  InterpolationIndexType = 0,
  StxBtreeIndexType,
  InterpolationIndexTypeV1,
};
