#pragma once

#include <iostream>

#include "utils.h"


typedef Uint64 BlockIDT;
typedef Uint64 RelOffsetT;

static const RelOffsetT INVALID_OFFSET = std::numeric_limits<RelOffsetT>::max();

static const Uint64 BLOCKID_BITS = 40;
static const Uint64 BLOCKOFFSET_BITS = 64 - BLOCKID_BITS;

class OffsetT {
public:
  OffsetT(const BlockIDT bid, const RelOffsetT rel_offset)
    : offset_((bid << (64 - BLOCKID_BITS)) + rel_offset) {}

  OffsetT(const Uint64 offset) : offset_(offset) {}

  OffsetT() : offset_(0) {}

  BlockIDT block_id() const { 
    return (offset_ >> (64 - BLOCKID_BITS));
  }

  RelOffsetT rel_offset() const {
    return ( (offset_ << BLOCKID_BITS) >> BLOCKID_BITS );
  }

  Uint64 raw_data() const {
    return offset_;
  }

  /// prints out OffsetT.
  friend std::ostream& operator<<(std::ostream& out, OffsetT const& offset) {
    out << offset.block_id() << "." << offset.rel_offset();
    return out;
  }

private:
  Uint64 offset_;
};
