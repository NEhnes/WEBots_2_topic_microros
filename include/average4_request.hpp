#ifndef AVERAGE4_SERVICE_HPP
#define AVERAGE4_SERVICE_HPP

#include <stdint.h>

struct Average4Request {
  int32_t input1;
  int32_t input2;
  int32_t input3;
  int32_t input4;
};

struct Average4Response {
  float output_value;
};

#endif