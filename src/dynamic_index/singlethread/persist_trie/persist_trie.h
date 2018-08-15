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

struct BufferedInnerNode : public BaseNode {

  void add_node(BaseNode *node) {
    nodes_.push_back(node);
  }

  std::vector<BaseNode*> nodes_;
};


struct LeafNode : public BaseNode {
  LeafNode(const size_t key_len) : key_len_(key_len), val_count_(0), val_capacity_(1) {
    size_t kvs_len = key_len + val_capacity_ * sizeof(uint64_t);
    kvs_ = new unsigned char[kvs_len];
    memset(kvs_, 0, kvs_len);
  }
  LeafNode(const unsigned char *key, const size_t key_len) : key_len_(key_len), val_count_(0), val_capacity_(1) {
    size_t kvs_len = key_len + val_capacity_ * sizeof(uint64_t);
    kvs_ = new unsigned char[kvs_len];
    memset(kvs_, 0, kvs_len);
    memcpy(kvs_, key, key_len);
  }

  LeafNode(const unsigned char *key, const size_t key_len, const uint64_t value) : key_len_(key_len), val_count_(1), val_capacity_(1) {
    size_t kvs_len = key_len + val_capacity_ * sizeof(uint64_t);
    kvs_ = new unsigned char[kvs_len];
    memset(kvs_, 0, kvs_len);
    memcpy(kvs_, key, key_len);
    memcpy(kvs_ + key_len, &value, sizeof(uint64_t));
  }

  ~LeafNode() {
    delete[] kvs_;
    kvs_ = nullptr;
  }

  void add_value(const uint64_t val) {
    if (val_count_ == val_capacity_) {
      unsigned char *old_kvs = kvs_;

      size_t old_kvs_len = key_len_ + val_capacity_ * sizeof(uint64_t);
      val_capacity_ *= 2;
      size_t kvs_len = key_len_ + val_capacity_ * sizeof(uint64_t);
      kvs_ = new unsigned char[kvs_len];
      memset(kvs_, 0, kvs_len);
      memcpy(kvs_, old_kvs, old_kvs_len);
      memcpy(kvs_ + old_kvs_len, &val, sizeof(uint64_t));

      delete[] old_kvs;
      old_kvs = nullptr;
    } else {
      memcpy(kvs_ + key_len_ + val_count_ * sizeof(uint64_t), &val, sizeof(uint64_t));
    }
    val_count_ ++;
  }

  uint32_t key_len_;
  uint32_t val_count_;
  uint32_t val_capacity_;
  unsigned char *kvs_;
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
      root_ = new BufferedInnerNode();
    } 
    LeafNode *leaf_node = new LeafNode(key, key_size, value);
    ((BufferedInnerNode*)root_)->add_node(leaf_node);
    return true;
  }

  bool find(const unsigned char *key, const size_t key_size, std::vector<uint64_t> &values) {
    if (root_ == nullptr) {
      return false;
    }
    std::vector<BaseNode*> &nodes = ((BufferedInnerNode*)root_)->nodes_;
    for (size_t i = 0; i < nodes.size(); ++i) {
      if (memcmp(((LeafNode*)nodes.at(i))->kvs_, key, key_size) == 0) {
        uint64_t ret_val;
        memcpy(&ret_val, ((LeafNode*)nodes.at(i))->kvs_ + key_size, key_size);
        values.push_back(ret_val);
        return true;
      }
    }

    return false;
  }




private:


  BaseNode *root_;
  Storage *storage_;
};