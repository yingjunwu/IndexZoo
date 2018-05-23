#pragma once

template<typename KeyT>
class BaseKeyGenerator {
public:
  
  BaseKeyGenerator() {}
  virtual ~BaseKeyGenerator() {}

  virtual KeyT get_next_key() = 0;

};
