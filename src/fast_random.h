#pragma once

#include <string>

// Fast random number generator
class FastRandom {
 public:

  FastRandom() : seed_(0) { set_seed0(seed_); }

  FastRandom(const uint64_t seed) : seed_(seed) { set_seed0(seed_); }

  template<typename T> 
  inline T next();

  /** [0.0, 1.0) */
  inline double next_uniform() {
    return ((((uint64_t)next(26)) << 27) + next(27)) / (double)(1L << 53);
  }

  inline char next_char() { return next(8) % 256; }

  inline char next_readable_char() {
    static const char readables[] =
        "0123456789@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
    return readables[next(6)];
  }

  inline void next_string(const size_t len, std::string &s) {
    s.resize(len, 0);
    for (size_t i = 0; i < len; i++) {
      s[i] = next_char();
    }
  }

  inline void next_readable_string(const size_t len, std::string &s) {
    s.resize(len, 0);
    for (size_t i = 0; i < len; i++) {
      s[i] = next_readable_char();
    }
  }

  inline void next_chars(const size_t len, char *s) {
    for (size_t i = 0; i < len; i++) {
      s[i] = next_char();
    }
  }

  inline void next_readable_chars(const size_t len, char *s) {
    for (size_t i = 0; i < len; i++) {
      s[i] = next_readable_char();
    }
  }

  inline uint64_t get_seed() { return seed_; }

  inline void set_seed(const uint64_t seed) { seed_ = seed; }

 private:
  inline void set_seed0(const uint64_t seed) {
    seed_ = (seed ^ 0x5DEECE66DL) & ((1L << 48) - 1);
  }

  inline uint32_t next(const unsigned int bits) {
    seed_ = (seed_ * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    return (uint32_t)(seed_ >> (48 - bits));
  }

  uint64_t seed_;
};

template<>
inline uint64_t FastRandom::next<uint64_t>() { return ((uint64_t)next(32) << 32) + next(32); }

template<>
inline uint32_t FastRandom::next<uint32_t>() { return next(32); }

template<>
inline uint16_t FastRandom::next<uint16_t>() { return (uint16_t)next(16); }

template<>
inline uint8_t FastRandom::next<uint8_t>() { return (uint8_t)next(8); }




