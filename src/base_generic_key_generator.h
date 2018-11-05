#pragma once

#include "generic_key.h"

class BaseGenericKeyGenerator {
public:

  BaseGenericKeyGenerator() {}
  virtual ~BaseGenericKeyGenerator() {}

  virtual void get_next_key(GenericKey &key) = 0;
};
