#pragma once

template<typename T>
void add(const T *lhs, const T *rhs, const size_t size, T *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = lhs[i] + rhs[i];
  }
}

template<typename T>
void multiply(const T *lhs, const T *rhs, const size_t size, T *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = lhs[i] * rhs[i];
  }
}

template<typename T>
void compare_eq(const T *data, const size_t size, const T value, bool *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = (data[i] == value);
  }
}

template<typename T>
void compare_lt(const T *data, const size_t size, const T value, bool *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = (data[i] < value);
  }
}

template<typename T>
void compare_le(const T *data, const size_t size, const T value, bool *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = (data[i] <= value);
  }
}

template<typename T>
void compare_gt(const T *data, const size_t size, const T value, bool *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = (data[i] > value);
  }
}

template<typename T>
void compare_ge(const T *data, const size_t size, const T value, bool *ret) {
  for (size_t i = 0; i < size; i++) {
    ret[i] = (data[i] >= value);
  }
}
