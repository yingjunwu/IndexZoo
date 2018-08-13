#pragma once


#include <cstring>
#include <string>
#include <vector>

#define LEAF_NODE   0
#define INNER_NODE  1



struct PersistStorage {

  
};


struct BaseNode {
  uint8_t node_type_;
  uint8_t num_childern_;
  uint32_t partial_len_;
};




class PersistTrie {


public:
  PersistTrie() : root_(nullptr) {}
  ~PersistTrie() {

  }


  bool insert(const unsigned char *key, const size_t key_size, const uint64_t value) {
    if (root_ == nullptr) {

    }
    return true;
  }

  bool find(const unsigned char *key, const size_t key_size, std::vector<uint64_t> &values) {
    return true;
  }




private:


  BaseNode *root_;
};