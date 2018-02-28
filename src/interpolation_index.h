#pragma once

#include <algorithm>

#include "base_index.h"


template<typename KeyT>
class InterpolationIndex : public BaseIndex<KeyT> {

  struct KeyValuePair {
    KeyValuePair(const KeyT key, const Uint64 value) : key_(key), value_(value) {}

    KeyT key_;
    Uint64 value_;
  };

  static bool compare_func(KeyValuePair &lhs, KeyValuePair &rhs) {
    return lhs.key_ < rhs.key_;
  }

public:
  InterpolationIndex() {}
  virtual ~InterpolationIndex() {}

  virtual void insert(const KeyT &key, const Uint64 &value) final {
    container_.emplace_back(KeyValuePair(key, value));
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {
    if (entry_count_ == 0) {
      return;
    }

    if (key > max_ || key < min_) {
      return;
    }

    if (max_ == min_) {
      if (max_ == key) {
        for (auto entry : container_) {
          values.push_back(entry.value_);
        }
      }
      return;
    }

    // guess where the data lives
    int guess = int((key - min_) * 1.0 / (max_ - min_) * (entry_count_ - 1));
    
    // if the guess is correct
    if (container_.at(guess).key_ == key) {
      values.push_back(container_.at(guess).value_);
      
      // move left
      int guess_lhs = guess - 1;
      while (guess_lhs >= 0) {

        if (container_.at(guess_lhs).key_ == key) {
          values.push_back(container_.at(guess_lhs).value_);
          guess_lhs -= 1;
        } else {
          break;
        }
      }
      // move right
      int guess_rhs = guess + 1;
      while (guess_rhs < entry_count_ - 1) {

        if (container_.at(guess_rhs).key_ == key) {
          values.push_back(container_.at(guess_rhs).value_);
          guess_rhs += 1;
        } else {
          break;
        }
      }
    }
    // if the guess is larger than the key
    else if (container_.at(guess).key_ > key) {
      // move left
      guess -= 1;
      while (guess >= 0) {

        if (container_.at(guess).key_ < key) {
          break;
        }
        else if (container_.at(guess).key_ > key) {
          guess -= 1;
          continue;
        } 
        else {
          values.push_back(container_.at(guess).value_);
          guess -= 1;
          continue;
        }
      }
    } 
    // if the guess is smaller than the key
    else {
      // move right
      guess += 1;
      while (guess < entry_count_ - 1) {

        if (container_.at(guess).key_ > key) {
          break;
        }
        else if (container_.at(guess).key_ < key) {
          guess += 1;
          continue;
        }
        else {
          values.push_back(container_.at(guess).value_);
          guess += 1;
          continue;
        }
      }
    }
    return;
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) final {
    assert(lhs_key < rhs_key);

    if (entry_count_ == 0) {
      return;
    }

    if (lhs_key > max_ || rhs_key < min_) {
      return;
    }

    if (max_ == min_) {
      if (max_ >= lhs_key && max_ <= rhs_key) {
        for (auto entry : container_) {
          values.push_back(entry.value_);
        }
      }
      return;
    }

    // guess where the data lives
    int guess = int((lhs_key - min_) * 1.0 / (max_ - min_) * (entry_count_ - 1));
    
    // if the guess is larger than or equal to lhs_key
    if (container_.at(guess).key_ >= lhs_key) {
      values.push_back(container_.at(guess).value_);
      
      // move left
      int guess_lhs = guess - 1;
      while (guess_lhs >= 0) {
        if (container_.at(guess_lhs).key_ >= lhs_key) {
          values.push_back(container_.at(guess_lhs).value_);
          guess_lhs -= 1;
        } else {
          break;
        }
      }
      // move right
      int guess_rhs = guess + 1;
      while (guess_rhs < entry_count_ - 1) {
        if (container_.at(guess_rhs).key_ <= rhs_key) {
          values.push_back(container_.at(guess_rhs).value_);
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
      while (guess < entry_count_ - 1) {
        if (container_.at(guess).key_ < lhs_key) {
          guess += 1;
          continue;
        }
        else if (container_.at(guess).key_ > rhs_key) {
          break;
        }
        else {
          values.push_back(container_.at(guess).value_);
          guess += 1;
          continue;
        }
      }
    }
    return;
  }

  virtual void scan(const KeyT &key, std::vector<Uint64> &values) final {
    for (size_t i = 0; i < container_.size(); ++i) {
      if (container_.at(i).key_ == key) {
        values.push_back(container_.at(i).value_);
      }
    }
  }

  virtual void scan_reverse(const KeyT &key, std::vector<Uint64> &values) final {
    for (int i = container_.size() - 1; i >= 0; --i) {
      if (container_.at(i).key_ == key) {
        values.push_back(container_.at(i).value_);
      }
    }
  }

  virtual void erase(const KeyT &key) final {
    // container_.erase(key);
  }

  virtual size_t size() const final {
    return container_.size();
  }

  virtual void reorganize() final {
    std::sort(container_.begin(), container_.end(), compare_func);
    min_ = container_.at(0).key_;
    max_ = container_.at(container_.size() - 1).key_;
    entry_count_ = container_.size();
  }

  virtual void print() const final {
    for (size_t i = 0; i < container_.size(); ++i) {
      std::cout << container_.at(i).key_ << " " << container_.at(i).value_ << std::endl;
    }
  }

private:
  std::vector<KeyValuePair> container_;
  KeyT min_;
  KeyT max_;
  size_t entry_count_;
};
