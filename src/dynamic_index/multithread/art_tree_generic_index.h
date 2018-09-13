#pragma once

#include "art_tree/Tree.h"

#include "base_dynamic_generic_index.h"
#include "data_table.h"
#include "utils.h"


namespace dynamic_index {
namespace multithread {


class ArtTreeGenericIndex : public BaseDynamicGenericIndex {

static void load_key_internal(void *ctx, TID tid, art::Key &tree_key) {

  auto data_table_ptr = reinterpret_cast<GenericDataTable*>(ctx);  

  char *key_ptr = data_table_ptr->get_tuple_key(OffsetT(tid));

  size_t key_len = strlen(key_ptr);
  size_t max_key_size = data_table_ptr->get_max_key_size();
  if (key_len > max_key_size) {
    key_len = max_key_size;
  }

  tree_key.setKeyLen(key_len);

  uint8_t *tree_key_data = &(tree_key[0]);
  memcpy(tree_key_data, key_ptr, key_len);
}

public:

  ArtTreeGenericIndex(GenericDataTable *table_ptr) : 
    BaseDynamicGenericIndex(table_ptr), 
    container_(load_key_internal, table_ptr), 
    ti_(container_.getThreadInfo()) {}
  
  virtual ~ArtTreeGenericIndex() {}

  virtual void insert(const GenericKey &key, const Uint64 &offset) final {

    art::Key tree_key;
    load_key(key, tree_key);

    bool rt = container_.insert(tree_key, offset, ti_);
  }

  virtual void find(const GenericKey &key, std::vector<Uint64> &offsets) final {

    art::Key tree_key;
    load_key(key, tree_key);

    bool rt = container_.lookup(tree_key, offsets, ti_);
  }

  virtual void find_range(const GenericKey &lhs_key, const GenericKey &rhs_key, std::vector<Uint64> &offsets) final {
    art::Key start_key, end_key;
    load_key(lhs_key, start_key);
    load_key(rhs_key, end_key);

    // Perform scan
    const uint32_t batch_size = 1000;
    std::vector<Uint64> tmp_result;

    art::Key curr_key;
    curr_key.setFrom(start_key);

    bool has_more = true;
    while (has_more) {
      art::Key next_key;
      has_more = container_.lookupRange(curr_key, end_key, next_key,
                                        tmp_result, batch_size, ti_);

      // Copy the results to the vector
      for (const auto &tid : tmp_result) {
        offsets.push_back(tid);
      }

      // Set the next key
      curr_key.setFrom(next_key);
    }

  }

  virtual void erase(const GenericKey &key) final {
    // container_.erase(key);
  }

  virtual size_t size() const final {

    // return art_size(&container_);
    return 0;
  }

private:
  void load_key(const GenericKey &key, art::Key &tree_key) {
    tree_key.setKeyLen(key.size());

    uint8_t *tree_key_data = &(tree_key[0]);
    memcpy(tree_key_data, key.raw(), key.size());

  }

private:
  art::Tree container_;
  art::ThreadInfo ti_;
};

}
}