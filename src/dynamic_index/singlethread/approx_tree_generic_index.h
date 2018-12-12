#pragma once

#include <unordered_map>

#include "base_dynamic_generic_index.h"

namespace dynamic_index {
namespace singlethread {

const size_t slice_len = 4;

class ApproxTreeGenericIndex : public BaseDynamicGenericIndex {

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
    for (int i = 0; i < key.size() / slice_len; ++i) {
      uint32_t tmp = *(uint32_t*)(key.raw() + slice_len * i);
      if (curr_node->children_.find(tmp) == curr_node->children_.end()) {
        curr_node->children_[tmp] = new InnerNode();
      }
      curr_node = curr_node->children_[tmp];
    }
    curr_node->values_.push_back( {key, offset} );
  }

  virtual void find(const GenericKey &key, std::vector<Uint64> &offsets) final {
    if (root_ == nullptr) { return; }
    InnerNode* curr_node = root_;
    for (int i = 0; i < key.size() / slice_len; ++i) {
      uint32_t tmp = *(uint32_t*)(key.raw() + slice_len * i);
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
  std::unordered_map<uint32_t, InnerNode*> children_;
};

private:
  InnerNode *root_;
};

}
}