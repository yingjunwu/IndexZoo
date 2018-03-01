#pragma once

#include "stx_btree_index.h"

#include "interpolation_index.h"
#include "interpolation_index_v1.h"
#include "interpolation_index_v2.h"

enum class IndexType {
  InterpolationIndexType = 0,
  InterpolationIndexTypeV1,
  InterpolationIndexTypeV2,
  StxBtreeIndexType,
};
