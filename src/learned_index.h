#pragma once

#include <iostream>
#include <algorithm>

template<typename KeyT>
bool compare_func(std::pair<KeyT, Uint64> &lhs, std::pair<KeyT, Uint64> &rhs) {
  return lhs.first < rhs.first;
}

template<typename KeyT>
class LearnedIndex {

public:
  LearnedIndex(const size_t size_hint) {}
  LearnedIndex() {}
  ~LearnedIndex() {}

  void insert(const KeyT &key, const Uint64 &value) {
    container_.emplace_back(std::pair<KeyT, Uint64>(key, value));
  }

  void find(const KeyT &key, std::vector<Uint64> &values) {
    if (entry_count_ == 0) {
      return;
    }

    if (max_ == min_) {
      if (max_ == key) {
        for (auto entry : container_) {
          values.push_back(entry.second);
        }
      }
      return;
    }

    // guess where the data lives
    int guess = ((key - min_) / (max_ - min_)) * (entry_count_ - 1);
    
    // if the guess is correct
    if (container_.at(guess).first == key) {
      values.push_back(container_.at(guess).second);
      
      // move left
      int guess_lhs = guess - 1;
      while (guess_lhs >= 0) {
        if (container_.at(guess_lhs).first == key) {
          values.push_back(container_.at(guess_lhs).second);
          guess_lhs -= 1;
        } else {
          break;
        }
      }
      // move right
      int guess_rhs = guess + 1;
      while (guess_rhs < entry_count_ - 1) {
        if (container_.at(guess_rhs).first == key) {
          values.push_back(container_.at(guess_rhs).second);
          guess_rhs += 1;
        } else {
          break;
        }
      }
    }
    // if the guess is larger than the key
    else if (container_.at(guess).first > key) {
      // move left
      guess -= 1;
      while (guess >= 0) {
        if (container_.at(guess).first < key) {
          break;
        }
        else if (container_.at(guess).first > key) {
          guess -= 1;
          continue;
        } 
        else {
          values.push_back(container_.at(guess).second);
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
        if (container_.at(guess).first > key) {
          break;
        }
        else if (container_.at(guess).first < key) {
          guess += 1;
          continue;
        }
        else {
          values.push_back(container_.at(guess).second);
          guess += 1;
          continue;
        }
      }
    }
    return;
  }

  void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) {
    assert(false);
  }

  void erase(const KeyT &key) {
    // container_.erase(key);
  }

  size_t size() const {
    return container_.size();
  }

  void reorganize() {
    std::sort(container_.begin(), container_.end(), compare_func<KeyT>);
    min_ = container_.at(0).first;
    max_ = container_.at(container_.size() - 1).first;
    entry_count_ = container_.size();
  }

  void print() {
    for (size_t i = 0; i < container_.size(); ++i) {
      std::cout << container_.at(i).first << " " << container_.at(i).second << std::endl;
    }
  }

private:
  std::vector<std::pair<KeyT, Uint64>> container_;
  KeyT min_;
  KeyT max_;
  size_t entry_count_;
};
