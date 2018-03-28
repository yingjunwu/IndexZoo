#pragma once

#include "art_tree/Tree.h"

#include "base_dynamic_index.h"
#include "data_table.h"


namespace dynamic_index {
namespace multithread {

template<typename KeyT, typename ValueT>
class ArtTreeIndex : public BaseDynamicIndex<KeyT, ValueT> {


static void load_key_internal(void *ctx, TID tid, art::Key &tree_key) {

  auto data_table_ptr = reinterpret_cast<DataTable<KeyT, ValueT>*>(ctx);  

  KeyT *key_ptr = data_table_ptr->get_tuple_key(OffsetT(tid));

  tree_key.setKeyLen(sizeof(KeyT));

  // uint8_t *tree_key_data = &(tree_key[0]);
  // memcpy(tree_key_data, key_ptr, sizeof(KeyT));

  reinterpret_cast<uint64_t *>(&tree_key[0])[0] = __builtin_bswap64(*key_ptr);
}

public:

  ArtTreeIndex(DataTable<KeyT, ValueT> *table_ptr) : 
    BaseDynamicIndex<KeyT, ValueT>(table_ptr), 
    container_(load_key_internal, table_ptr), 
    ti_(container_.getThreadInfo()) {}
  
  virtual ~ArtTreeIndex() {}

  virtual void insert(const KeyT &key, const Uint64 &value) final {

    art::Key tree_key;
    load_key(key, tree_key);

    bool rt = container_.insert(tree_key, value, ti_);
  }

  virtual void find(const KeyT &key, std::vector<Uint64> &values) final {

    art::Key tree_key;
    load_key(key, tree_key);

    bool rt = container_.lookup(tree_key, values, ti_);
  }

  virtual void find_range(const KeyT &lhs_key, const KeyT &rhs_key, std::vector<Uint64> &values) final {
    // assert(false);
  }

  virtual void erase(const KeyT &key) final {
    // container_.erase(key);
  }

  virtual size_t size() const final {

    // return art_size(&container_);
    return 0;
  }

private:
  void load_key(const KeyT &key, art::Key &tree_key) {
    tree_key.setKeyLen(sizeof(KeyT));

    // uint8_t *tree_key_data = &(tree_key[0]);
    // memcpy(tree_key_data, &key, sizeof(KeyT));

    reinterpret_cast<uint64_t *>(&tree_key[0])[0] = __builtin_bswap64(key);

  }

private:
  art::Tree container_;
  art::ThreadInfo ti_;
};

}
}