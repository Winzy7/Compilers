#include "copyright.h"

#ifndef _COOL_YAML_H_
#define _COOL_YAML_H_

struct token {
  int kind;
  unsigned int lineno;
  std::string symbol;
  bool boolean;
};

#endif
