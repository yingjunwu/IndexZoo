#pragma once

#include <map>
#include <unordered_map>

#include "base_dynamic_generic_index.h"
#include "slice.h"

namespace dynamic_index {
namespace singlethread {

const size_t default_slice_size = 4;
const size_t fanout = 64;

class ApproxTreeGenericIndex : public BaseDynamicGenericIndex {

private:
struct InnerNode {
  InnerNode() {}
  ~InnerNode() {
    for (auto& entry : children_) {
      if (entry.second == nullptr) {
        delete entry.second;
        entry.second = nullptr;
      }
    }
  }
  std::vector<std::pair<GenericKey, Uint64>> values_;
  std::map<uint32_t, InnerNode*> children_;
  size_t slice_size_;
};

public:
  ApproxTreeGenericIndex(GenericDataTable *table_ptr) : BaseDynamicGenericIndex(table_ptr), root_(nullptr) {}

  virtual ~ApproxTreeGenericIndex() {
    delete root_;
    root_ = nullptr;
  }
  
  virtual void insert(const GenericKey &key, const Uint64 &offset) final {
    if (root_ == nullptr) {
      root_ = new InnerNode();
    }
    InnerNode* curr_node = root_;
    // int offset = 0;
    // while (offset < key.size()) {

    // }
    
    for (int i = 0; i < key.size() / default_slice_size; ++i) {
      uint32_t tmp = *(uint32_t*)(key.raw() + default_slice_size * i);
      if (curr_node->children_.find(tmp) == curr_node->children_.end()) {
        curr_node->children_[tmp] = new InnerNode();
      }
      curr_node = curr_node->children_[tmp];
    }
    curr_node->values_.push_back( {key, offset} );
  }

  // void compact() {
  //   if (root_ == nullptr) { return; }
  //   compact_helper(root_);
  // }

  // void compact_helper(InnerNode* node) {
  //   if (node->children_.size() >= fanout) {
  //     do_compaction(node);
  //     return;
  //   }
  //   for (auto entry : node->children_) {
  //     compact_helper(entry.second);
  //   }
  // }

  // void do_compaction(InnerNode* node) {
  // }

  virtual void find(const GenericKey &key, std::vector<Uint64> &offsets) final {
    if (root_ == nullptr) { return; }
    InnerNode* curr_node = root_;
    for (int i = 0; i < key.size() / default_slice_size; ++i) {
      uint32_t tmp = *(uint32_t*)(key.raw() + default_slice_size * i);
      if (curr_node->children_.find(tmp) == curr_node->children_.end()) {
        return;
      }
      curr_node = curr_node->children_[tmp];
    }
    for (auto &entry : curr_node->values_) {
      offsets.push_back(entry.second);
    }
  }

  virtual void find_range(const GenericKey &lhs_key, const GenericKey &rhs_key, std::vector<Uint64> &offsets) final {
  }

  virtual void scan_full(std::vector<Uint64> &offsets, const size_t count) final {
  }

  virtual void erase(const GenericKey &key) final {
  }

  virtual size_t size() const final {
    return 0;
  }

  virtual void print() const final {
    print_helper(root_, 0);
  }

  void print_helper(InnerNode* node, const size_t depth) const {
    if (node == nullptr) { return; }
    std::cout << "depth: " << depth << " " << node->children_.size() << std::endl;
    for (auto entry : node->children_) {
      print_helper(entry.second, depth + 1);
    }
  }

private:
  InnerNode *root_;
};

}
}