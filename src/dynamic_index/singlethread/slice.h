#pragma once


template<size_t SliceSize>
struct Slice {

public:
  Slice() {}
  ~Slice() {}

private:
  char data_[SliceSize];
};