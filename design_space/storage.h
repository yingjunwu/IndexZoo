#pragma once

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <unistd.h>

const size_t BLOCK_SIZE = 4 * 1024;

class Storage {
public:
  Storage() : block_count_(0) {
    file_ = fopen("test", "w+b");
  }

  ~Storage() {
    fclose(file_);
  }

  size_t write_block(const char *data) {
    size_t ret = fwrite(data, BLOCK_SIZE, 1, file_);
    assert(ret == 1);

    fflush(file_);
    fsync(fileno(file_));
    size_t block_id = block_count_;
    block_count_++;
    return block_id;
  }

  void read_block(const size_t block_id, char *data) const {
    fseek(file_, block_id * BLOCK_SIZE, SEEK_SET);
    int ret = fread(data, BLOCK_SIZE, 1, file_);
    assert(ret == 1);
  }

private:
  FILE *file_;
  size_t block_count_;
};

