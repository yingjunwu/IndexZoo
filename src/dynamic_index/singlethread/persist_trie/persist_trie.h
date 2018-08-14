#pragma once


#include <cstring>
#include <string>
#include <vector>

#include "storage.h"

#define LEAF_NODE   0
#define INNER_NODE  1

#define MAX_PREFIX_LEN 10

struct BaseNode {
};

struct InnerNode : public BaseNode {
  uint8_t node_type_;
  uint8_t num_childern_;
  uint32_t prefix_len_;
  unsigned char partial_[MAX_PREFIX_LEN];
};



struct LeafNode : public BaseNode {
  uint32_t key_len_;
  uint32_t val_count_;
  uint32_t val_capacity_;
  unsigned char kvs_[];
};

class PersistTrie {

public:
  PersistTrie() : root_(nullptr) {
    storage_ = new Storage("persist_trie.dat");
  }
  ~PersistTrie() {

    delete storage_;
    storage_ = nullptr;
  }


  bool insert(const unsigned char *key, const size_t key_size, const uint64_t value) {
    if (root_ == nullptr) {
      roo
    }
    return true;
  }

  bool find(const unsigned char *key, const size_t key_size, std::vector<uint64_t> &values) {
    return true;
  }




private:


  BaseNode *root_;
  Storage *storage_;
};