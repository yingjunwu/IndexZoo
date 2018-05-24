#pragma once

#include <cstring>

template<size_t KeySize>
struct GenericComparator;

template<size_t KeySize>
struct GenericKey {

friend GenericComparator<KeySize>;
public:
  GenericKey() { memset(data_, 0, KeySize); }

  GenericKey(const char* data) { memcpy(data_, data, KeySize); }

  inline char* raw() { return data_; }

private:
  char data_[KeySize];
};


template<size_t KeySize>
struct GenericComparator {
public:
  GenericComparator() {}

  inline bool operator()(const GenericKey<KeySize> &lhs, const GenericKey<KeySize> &rhs) const {
    return strcmp(lhs.data_, rhs.data_) < 0;
  }
};

