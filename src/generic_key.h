#pragma once

#include <cstring>
#include <cassert>

#include "utils.h"
#include "cityhash.h"

struct GenericComparator;

struct GenericKey {

friend GenericComparator;

public:
  GenericKey() : data_(nullptr), data_size_(0) {}

  GenericKey(const size_t data_size) {
    if (data_size == 0) {
      data_ = nullptr;
      data_size_ = 0;
    } else {
      data_ = new char[data_size];
      memset(data_, 0, data_size);
      data_size_ = data_size;
    }
  }

  GenericKey(const char* data, const size_t data_size) { 
    if (data_size == 0) {
      data_size_ = 0;
      ASSERT(data == nullptr, "data must be nullptr");
    } else {
      data_ = new char[data_size];
      memcpy(data_, data, data_size);
      data_size_ = data_size;
    }
  }

  GenericKey(const GenericKey &key) {
    if (key.data_size_ == 0) {
      data_ = nullptr;
      data_size_ = 0;
    } else {
      data_ = new char[key.data_size_];
      memcpy(data_, key.data_, key.data_size_);
      data_size_ = key.data_size_;
    }
  }

  ~GenericKey() {
    delete[] data_;
    data_ = nullptr;
    data_size_ = 0;
  }

  GenericKey& operator=(const GenericKey &key) {
    if (data_ != nullptr) {
      ASSERT(data_size_ != 0, "data size cannot be 0");
      delete[] data_;
      data_size_ = 0;
    }
    data_ = new char[key.data_size_];
    memcpy(data_, key.data_, key.data_size_);
    data_size_ = key.data_size_;
    return *this;
  }

  inline char* raw() const { return data_; }

  inline size_t size() const { return data_size_; }

  // transfer ownership
  void reset(char *data, const size_t data_size) {
    ASSERT(data_ == nullptr && data_size_ == 0, "must be uninitiated");

    data_ = data;
    data_size_ = data_size;

  }

  void resize(const size_t data_size) {
    ASSERT(data_ == nullptr && data_size_ == 0, "must be uninitiated");
    
    if (data_size == 0) {
      data_ = nullptr;
      data_size_ = 0;
    } else {
      data_ = new char[data_size];
      memset(data_, 0, data_size);
      data_size_ = data_size;
    }
  }

  bool operator==(const GenericKey &rhs) const {
    if (data_size_ != rhs.data_size_) {
      return false;
    }
    int rt = memcmp(data_, rhs.data_, data_size_);
    if (rt == 0) {
      return true;
    } else {
      return false;
    }
  }

  bool operator<(const GenericKey &rhs) const {
    size_t cmp_len = (data_size_ < rhs.data_size_) ? data_size_ : rhs.data_size_;
    int rt = memcmp(data_, rhs.data_, cmp_len);
    if (rt == 0) {
      if (data_size_ < rhs.data_size_) {
        return true;
      } else {
        return false;
      }
    } else if (rt < 0) {
      return true;
    } else {
      return false;
    }
  }

  bool operator>(const GenericKey &rhs) const {
    size_t cmp_len = (data_size_ < rhs.data_size_) ? data_size_ : rhs.data_size_;
    int rt = memcmp(data_, rhs.data_, cmp_len);
    if (rt == 0) {
      if (data_size_ > rhs.data_size_) {
        return true;
      } else {
        return false;
      }
    } else if (rt > 0) {
      return true;
    } else {
      return false;
    }
  }

private:
  char *data_ = nullptr;
  size_t data_size_ = 0;
};

// "less than" relation
struct GenericKeyComparator {
  inline bool operator()(const GenericKey &lhs, const GenericKey &rhs) const {
    size_t cmp_len = (lhs.size() < rhs.size()) ? lhs.size() : rhs.size();
    int rt = memcmp(lhs.raw(), rhs.raw(), cmp_len);
    if (rt == 0) {
      if (lhs.size() < rhs.size()) {
        return true;
      } else {
        return false;
      }
    } else if (rt < 0) {
      return true;
    } else {
      return false;
    }

  }
};

struct GenericKeyEqualityChecker {
  inline bool operator()(const GenericKey &lhs, const GenericKey &rhs) const {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    int rt = memcmp(lhs.raw(), rhs.raw(), lhs.size());
    if (rt == 0) {
      return true;
    } else {
      return false;
    }
  }
};

struct GenericKeyHasher {
  inline std::size_t operator()(const GenericKey &key) const {
    return CityHash64(key.raw(), key.size());
  }
};

