#pragma once

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <unistd.h>

const size_t BLOCK_SIZE = 4 * 1024;

class OffsetT {
public:
  OffsetT(const uint64_t bid, const uint64_t rel_offset)
    : offset_((bid << 32) + rel_offset) {}

  OffsetT(const uint64_t offset) : offset_(offset) {}

  OffsetT() : offset_(0) {}

  uint64_t block_id() const { 
    return uint64_t(offset_ >> 32);
  }

  uint64_t rel_offset() const {
    return uint64_t(offset_ & 0xFFFFFFFF);
  }

  uint64_t raw_data() const {
    return offset_;
  }

  static uint64_t construct_raw_data(const uint64_t bid, const uint64_t rel_offset) {
    return uint64_t((bid << 32) + rel_offset);
  }

  /// prints out OffsetT.
  friend std::ostream& operator<<(std::ostream& out, OffsetT const& offset) {
    out << offset.block_id() << "." << offset.rel_offset();
    return out;
  }

private:
  uint64_t offset_;
};



class Storage {
public:
  Storage(const std::string &filename) : block_count_(0) {
    file_ = fopen(filename.c_str(), "w+b");
  }

  ~Storage() {
    fclose(file_);
  }

  uint64_t next_block_id() const {
    return block_count_;
  }

  uint64_t write_block(const char *data) {
    size_t ret = fwrite(data, BLOCK_SIZE, 1, file_);
    assert(ret == 1);

    fflush(file_);
    fsync(fileno(file_));
    
    uint64_t block_id = block_count_;
    block_count_++;
    return block_id;
  }

  void read_block(const uint64_t block_id, char *data) const {
    fseek(file_, block_id * BLOCK_SIZE, SEEK_SET);
    int ret = fread(data, BLOCK_SIZE, 1, file_);
    assert(ret == 1);
  }

private:
  FILE *file_;
  uint64_t block_count_;
};

