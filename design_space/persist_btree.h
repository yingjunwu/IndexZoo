#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <algorithm>

const size_t BLOCK_SIZE = 1024;

class Storage {
public:
  Storage() : block_count_(0) {
    file_ = fopen("test", "w+b");
  }

  ~Storage() {
    fclose(file_);
  }

  size_t write_block(const char *data) {
    size_t ret1 = fwrite(data, BLOCK_SIZE, 1, file_);
    fflush(file_);
    size_t ret = block_count_;
    block_count_++;
    return ret;
  }


  void read_data(const size_t block_id, char *data) const {
    fseek(file_, block_id * BLOCK_SIZE, SEEK_SET);
    int ret = fread(data, BLOCK_SIZE, 1, file_);
  }

  FILE *file_;
  size_t block_count_;
};


template<typename KeyT>
class PersistBTree {

public:
  PersistBTree(Storage *storage) : storage_(storage) {}
  ~PersistBTree() {}


  void insert(const KeyT key, const uint64_t value) {
    container_.push_back(std::pair<KeyT, uint64_t>(key, value));
  }

  void organize() {

  }

  std::vector<std::pair<KeyT, uint64_t>> container_;
  Storage *storage_;
};

