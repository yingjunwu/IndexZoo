#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <unistd.h>

// const size_t BLOCK_SIZE = 64 * 1024 * 1024;
const size_t BLOCK_SIZE = 4 * 1024;

class Storage {
public:
  Storage() : block_count_(0) {
    file_ = fopen("test", "w+b");
  }

  ~Storage() {
    fclose(file_);
  }

  size_t write_block(const char *data) {
    size_t ret = fwrite(data, BLOCK_SIZE, 1, file_);
    assert(ret == 1);

    fflush(file_);
    fsync(fileno(file_));
    size_t block_id = block_count_;
    block_count_++;
    return block_id;
  }

  void read_block(const size_t block_id, char *data) const {
    fseek(file_, block_id * BLOCK_SIZE, SEEK_SET);
    int ret = fread(data, BLOCK_SIZE, 1, file_);
    assert(ret == 1);
  }

private:
  FILE *file_;
  size_t block_count_;
};


template<typename KeyT>
class PersistVector {

typedef std::pair<KeyT, uint64_t> KVPair;
typedef std::pair<KeyT, KeyT> BoundPair;

static bool compare_func(KVPair &lhs, KVPair &rhs) {
  return lhs.first < rhs.first;
}

public:
  PersistVector(Storage *storage) : 
    storage_(storage), 
    block_(new char[BLOCK_SIZE]),
    is_persisted_(false) {}

  ~PersistVector() {
    delete[] block_;
    block_ = nullptr;
  }

  void insert(const KeyT key, const uint64_t value) {
    container_.push_back(KVPair(key, value));
  }

  // sort in-memory vector, persist to disk, and clean it up.
  void persist() {

    if (container_.size() == 0) {
      return;
    }

    std::sort(container_.begin(), container_.end(), compare_func);

    KeyT lower_bound, upper_bound;

    uint64_t curr_pos = sizeof(uint64_t);

    bool is_init = true;

    for (auto entry : container_) {

      if (curr_pos + sizeof(KeyT) + sizeof(uint64_t) > BLOCK_SIZE) {
        // if exceeds BLOCK_SIZE
        memcpy(block_, &curr_pos, sizeof(uint64_t));
        
        size_t block_id = storage_->write_block(block_);
        block_ids_.push_back(block_id);
        assert(block_bounds_.find(block_id) == block_bounds_.end());
        block_bounds_[block_id] = BoundPair(lower_bound, upper_bound);

        curr_pos = sizeof(uint64_t);
        is_init = true;
      }

      if (is_init == true) {
        lower_bound = entry.first;
        is_init = false;
      }

      memcpy(block_ + curr_pos, &(entry.first), sizeof(KeyT));
      curr_pos += sizeof(KeyT);
      memcpy(block_ + curr_pos, &(entry.second), sizeof(uint64_t));
      curr_pos += sizeof(uint64_t);

      upper_bound = entry.first;
    }

    memcpy(block_, &curr_pos, sizeof(uint64_t));

    size_t block_id = storage_->write_block(block_);
    block_ids_.push_back(block_id);
    assert(block_bounds_.find(block_id) == block_bounds_.end());
    block_bounds_[block_id] = BoundPair(lower_bound, upper_bound);
    
    container_.clear();
    is_persisted_ = true;
  }

  // fetch data from disk and store it in in-memory vector.
  void cache() {
    for (auto block_id : block_ids_) {

      storage_->read_block(block_id, block_);

      // read block
      uint64_t max_pos = 0;
      memcpy(&max_pos, block_, sizeof(uint64_t));
      uint64_t curr_pos = sizeof(uint64_t);
      
      while (curr_pos != max_pos) {
        assert(curr_pos < max_pos);

        KeyT key;
        uint64_t value;

        memcpy(&key, block_ + curr_pos, sizeof(KeyT));
        curr_pos += sizeof(KeyT);
        memcpy(&value, block_ + curr_pos, sizeof(uint64_t));
        curr_pos += sizeof(uint64_t);

        container_.push_back(KVPair(key, value));
      }

    }
    is_persisted_ = false;
  }

  void find(const KeyT key, std::vector<uint64_t> &values) {
    for (auto block_id : block_ids_) {
      
      storage_->read_block(block_id, block_);

      // read block
      uint64_t max_pos = 0;
      memcpy(&max_pos, block_, sizeof(uint64_t));
      uint64_t curr_pos = sizeof(uint64_t);
      

      while (curr_pos != max_pos) {
        assert(curr_pos < max_pos);

        KeyT load_key;
        uint64_t load_value;

        memcpy(&load_key, block_ + curr_pos, sizeof(KeyT));
        curr_pos += sizeof(KeyT);
        memcpy(&load_value, block_ + curr_pos, sizeof(uint64_t));
        curr_pos += sizeof(uint64_t);

        if (load_key > key) {
          return;
        }

        if (load_key == key) {
          values.push_back(load_value);
        }
      }
    }
  }

  void find1(const KeyT key, std::vector<uint64_t> &values) {
    for (auto block_id : block_ids_) {
      assert(block_bounds_.find(block_id) != block_bounds_.end());
      BoundPair bounds = block_bounds_.at(block_id);
      if (key < bounds.first) {
        return;
      }
      if (key > bounds.second) {
        continue;
      }
      
      // find tuple in block

      storage_->read_block(block_id, block_);

      // read block
      uint64_t max_pos = 0;
      memcpy(&max_pos, block_, sizeof(uint64_t));
      uint64_t curr_pos = sizeof(uint64_t);
      

      while (curr_pos != max_pos) {
        assert(curr_pos < max_pos);

        KeyT load_key;
        uint64_t load_value;

        memcpy(&load_key, block_ + curr_pos, sizeof(KeyT));
        curr_pos += sizeof(KeyT);
        memcpy(&load_value, block_ + curr_pos, sizeof(uint64_t));
        curr_pos += sizeof(uint64_t);

        if (load_key > key) {
          return;
        }

        if (load_key == key) {
          values.push_back(load_value);
        }
      }      

    }
  }

  void print() const {
    std::cout << "=================" << std::endl;
    std::cout << "is persisted: " << (is_persisted_ ? "true" : "false") << std::endl;
    std::cout << "number of elements in cache is: " << container_.size() << std::endl;

    size_t bound = 5;
    bound = std::min(container_.size(), bound);
    for (size_t i = 0; i < bound; ++i) {
      std::cout << container_.at(i).first << " " << container_.at(i).second << std::endl;
    }

    for (auto entry : block_bounds_) {
      std::cout << entry.first << " -> " << "(" << entry.second.first << " " << entry.second.second << ")" << std::endl;
    }

    std::cout << "=================" << std::endl;
  }
  
  
private:
  std::vector<KVPair> container_;
  Storage *storage_;
  char *block_; // buffer for holding to-be-persisted data
  std::vector<size_t> block_ids_;

  std::unordered_map<size_t, std::pair<KeyT, KeyT>> block_bounds_;

  bool is_persisted_;
};








