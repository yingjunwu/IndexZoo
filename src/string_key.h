#pragma once

#include <cstring>

template<size_t KeySize>
struct StringComparator;

template<size_t KeySize>
struct StringKey {

friend StringComparator<KeySize>;
public:
  StringKey() {}

  inline char* get_chars() { return data_; }

private:
  char data_[KeySize];
};


template<size_t KeySize>
struct StringComparator {
public:
  StringComparator() {}

  inline bool operator()(const StringKey<KeySize> &lhs, const StringKey<KeySize> &rhs) const {
    return strcmp(lhs.data_, rhs.data_) < 0;
  }
};

