#pragma once

#include <cstring>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <queue>

#include "base_dynamic_index.h"

namespace dynamic_index {
namespace singlethread {

template<typename KeyT, typename ValueT>
class BtreeIndex : public BaseDynamicIndex<KeyT, ValueT> {

//////////////////////////////////////////////////////////////////////
//// BEGIN INTERNAL DATA STRUCTURES
//////////////////////////////////////////////////////////////////////

// BRANCHING_FACTOR must be >= 2.
static const size_t BRANCHING_FACTOR = 2;

// INVALID_LEAF_OFFSET must be larger than BRANCHING_FACTOR.
static const size_t INVALID_LEAF_OFFSET = BRANCHING_FACTOR + 1;
  
  enum SearchType {
    SEARCH_EQ_TYPE = 0, // equal to
    SEARCH_LT_TYPE,     // less than
    SEARCH_GT_TYPE,     // greater than
    SEARCH_LE_TYPE,     // less than or equal to
    SEARCH_GE_TYPE,     // greater than or equal to
  };

  struct BaseNode {
    size_t node_id_;
    size_t node_size_;
    KeyT keys_[BRANCHING_FACTOR];
  };

  struct LeafNode {
    LeafNode(const size_t node_id) : node_id_(node_id), node_size_(0), next_leaf_(nullptr) {
      memset(keys_, 0, sizeof(KeyT) * BRANCHING_FACTOR);
      memset(values_, 0, sizeof(Uint64) * BRANCHING_FACTOR);
    }

    void set_next_leaf(LeafNode *next_leaf) {
      next_leaf_ = next_leaf;
    }

    LeafNode* get_next_leaf() const {
      return next_leaf_;
    }

    bool add(const KeyT key, const Uint64 value) {
      assert(node_size_ < BRANCHING_FACTOR);

      size_t offset = 0;
      for (; offset < node_size_; ++offset) {
        if (keys_[offset] > key) {
          break;
        }
      }
      if (offset != node_size_) {
        memmove(keys_ + offset + 1, keys_ + offset, sizeof(KeyT) * (node_size_ - offset));
        memmove(values_ + offset + 1, values_ + offset, sizeof(Uint64) * (node_size_ - offset));
      }

      keys_[offset] = key;
      values_[offset] = value;
      
      ++node_size_;

      if (node_size_ == BRANCHING_FACTOR) {
        // splitting required.
        return true;
      } else {
        return false;
      }
    }

    size_t find_in_node_eq(const KeyT key) const {
      for (size_t offset = 0; offset < node_size_; ++offset) {
        if (keys_[offset] == key) {
          return offset;
        }
      }
      return INVALID_LEAF_OFFSET;
    }

    void print_node() const {
      std::cout << "node id = " << node_id_ << std::endl;
      for (size_t i = 0; i < node_size_; ++i) {
        std::cout << keys_[i] << " : " << values_[i];
        if (i != node_size_ - 1) {
          std::cout << ", ";
        }
      }
      std::cout << std::endl;
    }

    size_t get_size() const {
      return node_size_;
    }

    size_t get_capacity() const {
      return BRANCHING_FACTOR;
    }

    size_t node_id_;
    size_t node_size_;
    KeyT keys_[BRANCHING_FACTOR];
    Uint64 values_[BRANCHING_FACTOR];
    LeafNode *next_leaf_;
  };

  struct InnerNode {
    InnerNode(const size_t node_id) : node_id_(node_id), node_size_(0) {
      memset(keys_, 0, sizeof(KeyT) * BRANCHING_FACTOR);
      memset(children_, 0, sizeof(void*) * (BRANCHING_FACTOR + 1));
    }

    bool add(KeyT key, void *lhs, void *rhs) {
      assert(node_size_ == 0);

      keys_[0] = key;
      children_[0] = lhs;
      children_[1] = rhs;

      ++node_size_;

      return false; // split not required.
    }

    bool add(KeyT key, void *node) {
      ASSERT(node_size_ != 0 && node_size_ < BRANCHING_FACTOR, "node size = " << node_size_);
      
      size_t offset = 0;
      for (; offset < node_size_; ++offset) {
        if (keys_[offset] > key) {
          break;
        }
      }
      memmove(keys_ + offset + 1, keys_ + offset, sizeof(KeyT) * (node_size_ - offset));
      memmove(children_ + offset + 1, children_ + offset, sizeof(void*) * (node_size_ - offset + 1));

      keys_[node_size_] = key;
      children_[node_size_] = node;

      ++node_size_;

      if (node_size_ == BRANCHING_FACTOR) {
        // splitting required.
        return true;
      } else {
        return false;
      }
    }

    void* find_in_node(const KeyT key) const {
      size_t offset;
      for (offset = 0; offset < node_size_; ++offset) {
        if (keys_[offset] > key) {
          break;
        }
      }
      return children_[offset];
    }

    void print_node() const {
      std::cout << "node id = " << node_id_ << std::endl;
      for (size_t i = 0; i < node_size_; ++i) {
        std::cout << keys_[i] << " -> " << ((BaseNode*)(children_[i]))->node_id_;
        if (i != node_size_ - 1) {
          std::cout << ", ";
        }
      }
      std::cout << std::endl;
    }

    size_t node_id_;
    size_t node_size_;
    KeyT keys_[BRANCHING_FACTOR];
    void* children_[BRANCHING_FACTOR + 1];
  };


  struct Cursor {
    Cursor() : node_(nullptr), offset_(INVALID_LEAF_OFFSET) {}

    LeafNode *node_;
    size_t offset_;
  };


//////////////////////////////////////////////////////////////////////
//// BEGIN MEMBER FUNCTIONS
//////////////////////////////////////////////////////////////////////

public:
  BtreeIndex(DataTable<KeyT, ValueT> *table_ptr) : BaseDynamicIndex<KeyT, ValueT>(table_ptr), root_(nullptr), tree_depth_(0), inc_node_id_(0) {}

  virtual ~BtreeIndex() {}

public:

  virtual void insert(const KeyT &key, const Uint64 &value) final {
    if (root_ == nullptr) {
      // this is an empty tree.
      // start with a leaf node.
      LeafNode *tmp_leaf = new LeafNode(get_inc_node_id());

      bool rt = tmp_leaf->add(key, value);
      assert(rt == false); // return false means no splitting is required.
      root_ = (void*)tmp_leaf;

      tree_depth_++;

      return;
    }

    // pass root_ into recursive_insert
    bool is_splitting = recursive_insert(root_, key, value, 0);

    if (is_splitting == false) {
      return;
    }

    // if splitting is required, then split the root_ node.
    assert(tree_depth_ >= 1);

    if (tree_depth_ == 1) {
      // in this case, root_ is a leaf node.
      LeafNode *leaf_lhs = (LeafNode*)root_;
      LeafNode *leaf_rhs = new LeafNode(get_inc_node_id());
      leaf_rhs->set_next_leaf(leaf_lhs->get_next_leaf());
      leaf_lhs->set_next_leaf(leaf_rhs);
      
      // the split_key is equal to the 0th key held in leaf_rhs
      KeyT split_key = split_leaf_node(leaf_lhs, leaf_rhs);
      
      // must create a new inner node to hold the two leaves.
      InnerNode *tmp_inner = new InnerNode(get_inc_node_id());

      bool rt = tmp_inner->add(split_key, leaf_lhs, leaf_rhs);
      
      assert(rt == false); // return false means no splitting is required.

      root_ = (void*)tmp_inner;

      tree_depth_++;

      return;
    
    } else {
      // in this case, root_ is an internal node.
      InnerNode *inner_lhs = (InnerNode*)root_;
      InnerNode *inner_rhs = new InnerNode(get_inc_node_id());

      KeyT split_key = split_inner_node(inner_lhs, inner_rhs);

      // must create a new inner node to hold the two inner nodes.
      InnerNode *tmp_inner = new InnerNode(get_inc_node_id());

      bool rt = tmp_inner->add(split_key, inner_lhs, inner_rhs);

      assert(rt == false); // return false means no splitting is required.

      root_ = (void*)tmp_inner;

      tree_depth_++;

      return;

    }
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {
    if (root_ == nullptr) {
      return;
    }

    Cursor lower_bound_cursor;
    
    recursive_search(key, root_, 0, lower_bound_cursor);
    
    if (lower_bound_cursor.node_ != nullptr) {
      
      assert(lower_bound_cursor.offset_ != INVALID_LEAF_OFFSET);

      LeafNode *leaf_node = lower_bound_cursor.node_;
      size_t offset = lower_bound_cursor.offset_;

      values.push_back(leaf_node->values_[offset]);

      ++offset;
      
      bool is_finished = false;

      while (is_finished == false) {

        for (size_t i = offset; i < BRANCHING_FACTOR; ++i) {
          if (leaf_node->keys_[i] == key) {

            values.push_back(leaf_node->values_[offset]);

          } else {
            is_finished = true;
            break;
          }
        } // finished iterating the current leaf node.

        // mismatch found
        if (is_finished == true) {
          break;
        }

        // no more leaf node
        if (leaf_node->next_leaf_ == nullptr) {
          break;
        }

        leaf_node = leaf_node->next_leaf_;
        offset = 0;
      }

      return;

    } else {
      // if the cursor points to nullptr, then directly return false.
      return;
    }

  }


  virtual void find_range(const KeyT &key_lhs, const KeyT &key_rhs, std::vector<Uint64> &values) final {
    if (root_ == nullptr) {
      return;
    }

    // Cursor lower_bound_cursor;
    
    // recursive_search(key, root_, 0, lower_bound_cursor);
    
    // if (lower_bound_cursor.node_ != nullptr) {
      
    //   assert(lower_bound_cursor.offset_ != INVALID_LEAF_OFFSET);

    //   LeafNode *leaf_node = lower_bound_cursor.node_;
    //   size_t offset = lower_bound_cursor.offset_;

    //   // buffer.size_ == 0
    //   // buffer.values_[buffer.size_] = leaf_node->values_[offset];
    //   // ++buffer.size_;

    //   values.push_back(leaf_node->values_[offset]);

    //   ++offset;
      
    //   bool is_finished = false;

    //   while (is_finished == false) {

    //     for (size_t i = offset; i < BRANCHING_FACTOR; ++i) {
    //       if (leaf_node->keys_[i] == key) {

    //         // a new value found. add to return values.

    //         values.push_back(leaf_node->values_[offset]);


    //       } else {
    //         is_finished = true;
    //         break;
    //       }
    //     } // finished iterating the current leaf node.

    //     // mismatch found
    //     if (is_finished == true) {
    //       break;
    //     }

    //     // no more leaf node
    //     if (leaf_node->next_leaf_ == nullptr) {
    //       break;
    //     }

    //     leaf_node = leaf_node->next_leaf_;
    //     offset = 0;
    //   }

    //   return true;

    // } else {
    //   // if the cursor points to nullptr, then directly return false.
    //   return false;
    // }
  }

  virtual void erase(const KeyT &key) final {}

  virtual size_t size() const final {
    return 0;
  }

  virtual void print() const final {
    if (root_ == nullptr) {
      std::cout << "this is an empty tree!" << std::endl;
      return;
    }

    std::queue<std::pair<void*, size_t>> queue;

    queue.push(std::make_pair(root_, 0));

    while(queue.empty() != true) {

      print_node(queue);
    }

  }

private:
  // split leaf node into two parts.
  // the size of first part is BRANCHING_FACTOR / 2.
  // the size of second part is BRANCHING_FACTOR - left_size.
  KeyT split_leaf_node(LeafNode *leaf_lhs, LeafNode *leaf_rhs) {

    size_t left_size = BRANCHING_FACTOR / 2;
    size_t right_size = BRANCHING_FACTOR - left_size;

    assert(left_size >= 1 && right_size >= 1);

    memcpy(leaf_rhs->keys_, leaf_lhs->keys_ + left_size, sizeof(KeyT) * right_size);
    memcpy(leaf_rhs->values_, leaf_lhs->values_ + left_size, sizeof(Uint64) * right_size);

    leaf_lhs->node_size_ = left_size;
    leaf_rhs->node_size_ = right_size;

    std::cout << "left size = " << left_size << ", right size = " << right_size << std::endl;

    return leaf_rhs->keys_[0];
  }

  // split inner node into two parts.
  // the size of first part is BRANCHING_FACTOR / 2.
  // the size of second part is branching_factor - left_size - 1.
  // the inner entry will be moved to the new parent inner node.
  KeyT split_inner_node(InnerNode *inner_lhs, InnerNode *inner_rhs) {

    size_t left_key_size = BRANCHING_FACTOR / 2;
    size_t right_key_size = BRANCHING_FACTOR - left_key_size - 1;

    size_t left_value_size = left_key_size + 1;
    size_t right_value_size = right_key_size + 1;

    // assert(left_key_size >= 1 && right_key_size >= 1);

    memcpy(inner_rhs->keys_, inner_lhs->keys_ + left_key_size + 1, sizeof(KeyT) * right_key_size);
    memcpy(inner_rhs->children_, inner_lhs->children_ + left_value_size, sizeof(void*) * right_value_size);

    inner_lhs->node_size_ = left_key_size;
    inner_rhs->node_size_ = right_key_size;

    return inner_lhs->keys_[left_key_size];
  }


  /**
   * @param[in] node   targeted node
   * @param[in] key    key of the to-be-inserted entry
   * @param[in] value  value of the to-be-inserted entry
   * @param[in] depth  the depth of the targeted node
   *
   * return whether the targeted node needs splitting
   */
  bool recursive_insert(void *node, const KeyT &key, const Uint64 &value, const size_t depth) {
    assert(node != nullptr);
    if (depth == tree_depth_ - 1) {
      // must be a leaf node.
      return ((LeafNode*)node)->add(key, value);

    } else {
      // must be an inner node.
      void *child = ((InnerNode*)node)->find_in_node(key);
      bool rt = recursive_insert(child, key, value, depth + 1);

      if (rt == false) {
        // no node splitting is required. directly return false.
        return false;
      }

      // node splitting is required
      if (depth == tree_depth_ - 2) {
        // if child is leaf node.
        LeafNode *leaf_lhs = (LeafNode*)child;
        LeafNode *leaf_rhs = new LeafNode(get_inc_node_id());
        leaf_rhs->set_next_leaf(leaf_lhs->get_next_leaf());
        leaf_lhs->set_next_leaf(leaf_rhs);

        // the split_key is equal to the 0th key held in leaf_rhs
        KeyT split_key = split_leaf_node(leaf_lhs, leaf_rhs);

        return ((InnerNode*)node)->add(split_key, leaf_rhs);

      } else {
        // if child is inner node.
        InnerNode *inner_lhs = (InnerNode*)root_;
        InnerNode *inner_rhs = new InnerNode(get_inc_node_id());
    
        KeyT split_key = split_inner_node(inner_lhs, inner_rhs);

        return ((InnerNode*)node)->add(split_key, inner_rhs);

      }
    
    }
  }

  bool recursive_search(const KeyT &key, void *node, const size_t depth, Cursor &cursor) const {
    assert(node != nullptr);
    if (depth == tree_depth_ - 1) {
      // this is a leaf node. return cursor if found.
      LeafNode *leaf_node = (LeafNode*)node;
      size_t offset = leaf_node->find_in_node_eq(key);

      if (offset != INVALID_LEAF_OFFSET) {
        cursor.node_ = leaf_node;
        cursor.offset_ = offset;
      }

      return true;
    }

    InnerNode *inner_node = (InnerNode*)node;
    void* child = inner_node->find_in_node(key);
    assert(child != nullptr);

    recursive_search(key, child, depth + 1, cursor);

  }

  void print_node(std::queue<std::pair<void*, size_t>> &node_queue) const {
    void *node = node_queue.front().first;
    size_t depth = node_queue.front().second;
    if (depth == tree_depth_ - 1) {
      // handle leaf node
      LeafNode *leaf_node = (LeafNode*)node;
      std::cout << ":::LEAF NODE LEVEL = " << depth << std::endl;
      leaf_node->print_node();
      std::cout << "==================== " << std::endl;
      node_queue.pop();
    } else {
      // handle inner node
      InnerNode *inner_node = (InnerNode*)node;
      std::cout << ":::INNER NODE LEVEL = " << depth << std::endl;
      ((InnerNode*)node)->print_node();
      std::cout << "==================== " << std::endl;
      for (size_t i = 0; i < inner_node->node_size_ + 1; ++i) {
        node_queue.push(std::make_pair(inner_node->children_[i], depth + 1));
      }

      node_queue.pop();
    }
  }

  size_t get_inc_node_id() {
    size_t ret = inc_node_id_;
    ++inc_node_id_;
    return ret;
  }

private:
  void *root_;
  size_t tree_depth_;
  size_t inc_node_id_;

};


}
}
