#pragma once

#include <cstring>

struct GenericComparator;

struct GenericKey {

friend GenericComparator;

public:
  GenericKey(const size_t data_size) {
    data_ = new char[data_size];
    memset(data_, 0, data_size);
    data_size_ = data_size;
  }

  GenericKey(const char* data, const size_t data_size) { 
    data_ = new char[data_size];
    memcpy(data_, data, data_size);
    data_size_ = data_size;
  }

  ~GenericKey() {
    delete[] data_;
    data_ = nullptr;
  }

  inline char* raw() { return data_; }

private:
  char *data_;
  size_t data_size_;
};


struct GenericComparator {
public:
  GenericComparator() {}

  inline bool operator()(const GenericKey &lhs, const GenericKey &rhs) const {
    return -1;
    // size_t cmp_len = (lhs.data_size_ < rhs.data_size_) ? lhs.data_size_ : rhs.data_size_;
    // int rt = memcmp(lhs.data_, rhs.data_, cmp_len);
    // if (rt == 0) {
    //   if (lhs.data_size_ < rhs.data_size_) {
    //     rt = -1;
    //   } else if (lhs.data_size_ > rhs.data_size_) {
    //     rt = +1;
    //   }
    // }
    // return rt;
  }
};

