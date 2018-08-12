#pragma once

#include <cstring>
#include <cassert>

#include "utils.h"

struct GenericComparator;

struct GenericKey {

friend GenericComparator;

public:
  GenericKey() : data_(nullptr), data_size_(0) {}

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

  GenericKey(const GenericKey &key) {
    data_ = new char[key.data_size_];
    memcpy(data_, key.data_, key.data_size_);
    data_size_ = key.data_size_;
  }

  ~GenericKey() {
    delete[] data_;
    data_ = nullptr;
  }

  GenericKey& operator=(const GenericKey &key) {
    // if (data_ != nullptr) {
    //   ASSERT(data_size_ != 0, "data size is not 0: " << data_size_);
    //   delete[] data_;
    // }
    // ASSERT(data_ == nullptr, "member variables must be uninitiated");
    data_ = new char[key.data_size_];
    memcpy(data_, key.data_, key.data_size_);
    data_size_ = key.data_size_;
    return *this;
  }

  inline char* raw() const { return data_; }

  inline size_t size() const { return data_size_; }

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
  char *data_;
  size_t data_size_;
};


struct GenericComparator {
public:
  GenericComparator() {}

  inline bool operator()(const GenericKey &lhs, const GenericKey &rhs) const {
    size_t cmp_len = (lhs.data_size_ < rhs.data_size_) ? lhs.data_size_ : rhs.data_size_;
    int rt = memcmp(lhs.data_, rhs.data_, cmp_len);
    if (rt == 0) {
      if (lhs.data_size_ < rhs.data_size_) {
        rt = -1;
      } else if (lhs.data_size_ > rhs.data_size_) {
        rt = +1;
      }
    }
    return rt;
  }
};



// class Slice {
//  public:
//   // Create an empty slice.
//   Slice() : data_(""), size_(0) { }

//   // Create a slice that refers to d[0,n-1].
//   Slice(const char* d, size_t n) : data_(d), size_(n) { }

//   // Create a slice that refers to the contents of "s"
//   Slice(const std::string& s) : data_(s.data()), size_(s.size()) { }

//   // Create a slice that refers to s[0,strlen(s)-1]
//   Slice(const char* s) : data_(s), size_(strlen(s)) { }

//   // Return a pointer to the beginning of the referenced data
//   const char* data() const { return data_; }

//   // Return the length (in bytes) of the referenced data
//   size_t size() const { return size_; }

//   // Return true iff the length of the referenced data is zero
//   bool empty() const { return size_ == 0; }

//   // Return the ith byte in the referenced data.
//   // REQUIRES: n < size()
//   char operator[](size_t n) const {
//     assert(n < size());
//     return data_[n];
//   }

//   // Change this slice to refer to an empty array
//   void clear() { data_ = ""; size_ = 0; }

//   // Drop the first "n" bytes from this slice.
//   void remove_prefix(size_t n) {
//     assert(n <= size());
//     data_ += n;
//     size_ -= n;
//   }

//   // Return a string that contains the copy of the referenced data.
//   std::string ToString() const { return std::string(data_, size_); }

//   // Three-way comparison.  Returns value:
//   //   <  0 iff "*this" <  "b",
//   //   == 0 iff "*this" == "b",
//   //   >  0 iff "*this" >  "b"
//   int compare(const Slice& b) const;

//   // Return true iff "x" is a prefix of "*this"
//   bool starts_with(const Slice& x) const {
//     return ((size_ >= x.size_) &&
//             (memcmp(data_, x.data_, x.size_) == 0));
//   }

//  private:
//   const char* data_;
//   size_t size_;

//   // Intentionally copyable
// };

// inline bool operator==(const Slice& x, const Slice& y) {
//   return ((x.size() == y.size()) &&
//           (memcmp(x.data(), y.data(), x.size()) == 0));
// }

// inline bool operator!=(const Slice& x, const Slice& y) {
//   return !(x == y);
// }

// inline int Slice::compare(const Slice& b) const {
//   const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
//   int r = memcmp(data_, b.data_, min_len);
//   if (r == 0) {
//     if (size_ < b.size_) r = -1;
//     else if (size_ > b.size_) r = +1;
//   }
//   return r;
// }

