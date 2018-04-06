#pragma once

template<typename KeyT>
class BaseKeyGenerator {
public:
  
  BaseKeyGenerator() {}
  virtual ~BaseKeyGenerator() {}

  virtual KeyT get_insert_key() = 0;

  virtual KeyT get_read_key() = 0;
 
};
