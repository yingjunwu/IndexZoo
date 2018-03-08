#pragma once

#include <algorithm>
#include <cmath>

#include "base_index.h"


template<typename KeyT>
class InterpolationIndexV1 : public BaseIndex<KeyT> {

  struct Stats {

    Stats() : 
      is_first_match_(true), 
      find_op_count_(0), 
      find_op_profile_count_(0), 
      find_op_guess_distance_(0) {}

    void increment_find_op_counter() {
      find_op_count_++;
      is_first_match_ = true;
    }

    void measure_find_op_guess_distance(const int64_t guess_pos, const int64_t find_pos) {
      // record guess distance
      if (find_op_count_ % 1000 == 0) {
        if (is_first_match_ == true) {
          find_op_guess_distance_ += std::abs(guess_pos - find_pos);
          find_op_profile_count_ += 1;
          is_first_match_ = false;
        }
      }
    }

    bool is_first_match_;
    uint64_t find_op_count_;
    uint64_t find_op_profile_count_;
    uint64_t find_op_guess_distance_;
  };

  struct KeyValuePair {
    KeyValuePair() : key_(0), value_(0) {}
    KeyValuePair(const KeyT key, const Uint64 value) : key_(key), value_(value) {}

    KeyT key_;
    Uint64 value_;
  };

  static bool compare_func(KeyValuePair &lhs, KeyValuePair &rhs) {
    return lhs.key_ < rhs.key_;
  }

public:
  InterpolationIndexV1(const size_t size_hint = 1000) : size_(0), capacity_(size_hint) {
    container_ = new KeyValuePair[capacity_];
  }

  virtual ~InterpolationIndexV1() {
    delete[] container_;
    container_ = nullptr;
  }

  virtual void insert(const KeyT &key, const Uint64 &value) final {
    if (size_ >= capacity_) {
      KeyValuePair *old_container = container_;
      container_ = new KeyValuePair[capacity_ * 2];
      memcpy(container_, old_container, sizeof(KeyValuePair) * capacity_);
      capacity_ *= 2;
      delete[] old_container;
      old_container = nullptr;
    } 
    container_[size_].key_ = key;
    container_[size_].value_ = value;
    ++size_;
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {

    stats_.increment_find_op_counter();

    if (size_ == 0) {
      return;
    }

    if (key > max_ || key < min_) {
      return;
    }

    if (max_ == min_) {
      if (max_ == key) {
        for (size_t i = 0; i < size_; ++i) {
          values.push_back(container_[i].value_);
        }
      }
      return;
    }

    // guess where the data lives
    int guess = int((key - min_) * 1.0 / (max_ - min_) * (size_ - 1));

    int origin_guess = guess;
    
    // if the guess is correct
    if (container_[guess].key_ == key) {

      stats_.measure_find_op_guess_distance(origin_guess, guess);

      values.push_back(container_[guess].value_);
      
      // move left
      int guess_lhs = guess - 1;
      while (guess_lhs >= 0) {

        if (container_[guess_lhs].key_ == key) {
          values.push_back(container_[guess_lhs].value_);
          guess_lhs -= 1;
        } else {
          break;
        }
      }
      // move right
      int guess_rhs = guess + 1;
      while (guess_rhs < size_ - 1) {

        if (container_[guess_rhs].key_ == key) {
          values.push_back(container_[guess_rhs].value_);
          guess_rhs += 1;
        } else {
          break;
        }
      }
    }
    // if the guess is larger than the key
    else if (container_[guess].key_ > key) {
      // move left
      guess -= 1;
      while (guess >= 0) {

        if (container_[guess].key_ < key) {
          break;
        }
        else if (container_[guess].key_ > key) {
          guess -= 1;
          continue;
        } 
        else {

          stats_.measure_find_op_guess_distance(origin_guess, guess);

          values.push_back(container_[guess].value_);
          guess -= 1;
          continue;
        }
      }
    } 
    // if the guess is smaller than the key
    else {
      // move right
      guess += 1;
      while (guess < size_ - 1) {

        if (container_[guess].key_ > key) {
          break;
        }
        else if (container_[guess].key_ < key) {
          guess += 1;
          continue;
        }
        else {

          stats_.measure_find_op_guess_distance(origin_guess, guess);

          values.push_back(container_[guess].value_);
          guess += 1;
          continue;
        }
      }
    }
    return;
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) final {
    assert(lhs_key < rhs_key);

    if (size_ == 0) {
      return;
    }

    if (lhs_key > max_ || rhs_key < min_) {
      return;
    }

    if (max_ == min_) {
      if (max_ >= lhs_key && max_ <= rhs_key) {
        for (size_t i = 0; i < size_; ++i) {
          values.push_back(container_[i].value_);
        }
      }
      return;
    }

    // guess where the data lives
    int guess = int((lhs_key - min_) * 1.0 / (max_ - min_) * (size_ - 1));
    
    // if the guess is larger than or equal to lhs_key
    if (container_[guess].key_ >= lhs_key) {
      values.push_back(container_[guess].value_);
      
      // move left
      int guess_lhs = guess - 1;
      while (guess_lhs >= 0) {
        if (container_[guess_lhs].key_ >= lhs_key) {
          values.push_back(container_[guess_lhs].value_);
          guess_lhs -= 1;
        } else {
          break;
        }
      }
      // move right
      int guess_rhs = guess + 1;
      while (guess_rhs < size_ - 1) {
        if (container_[guess_rhs].key_ <= rhs_key) {
          values.push_back(container_[guess_rhs].value_);
          guess_rhs += 1;
        } else {
          break;
        }
      }
    }
    // if the guess is smaller than lhs_key
    else {
      // move right
      guess += 1;
      while (guess < size_ - 1) {
        if (container_[guess].key_ < lhs_key) {
          guess += 1;
          continue;
        }
        else if (container_[guess].key_ > rhs_key) {
          break;
        }
        else {
          values.push_back(container_[guess].value_);
          guess += 1;
          continue;
        }
      }
    }
    return;
  }

  virtual void scan(const KeyT &key, std::vector<Uint64> &values) final {
    for (size_t i = 0; i < size_; ++i) {
      if (container_[i].key_ == key) {
        values.push_back(container_[i].value_);
      }
    }
  }

  virtual void scan_reverse(const KeyT &key, std::vector<Uint64> &values) final {
    for (int i = size_ - 1; i >= 0; --i) {
      if (container_[i].key_ == key) {
        values.push_back(container_[i].value_);
      }
    }
  }

  virtual void erase(const KeyT &key) final {
    // container_.erase(key);
  }

  virtual size_t size() const final {
    return size_;
  }

  virtual void reorganize() final {
    std::sort(container_, container_ + size_, compare_func);
    min_ = container_[0].key_;
    max_ = container_[size_ - 1].key_;
  }

  virtual void print() const final {
    for (size_t i = 0; i < size_; ++i) {
      std::cout << container_[i].key_ << " " << container_[i].value_ << std::endl;
    }
  }

  virtual void print_stats() const final {
    std::cout << "average guess distance = " << stats_.find_op_guess_distance_ * 1.0 / stats_.find_op_profile_count_ << std::endl;
  }

private:
  KeyValuePair *container_;
  size_t size_;
  size_t capacity_;

  KeyT min_;
  KeyT max_;

  Stats stats_;
};
