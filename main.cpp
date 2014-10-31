#include <stdlib.h>
#include "int_map_tests.h"
#include "int_set_tests.h"
#include "string_map_tests.h"
#include "string_set_tests.h"

int main()
{
  if (!int_map_tests()) {
    return -1;
  }

  if (!int_set_tests()) {
    return -1;
  }

  if (!string_map_tests()) {
    return -1;
  }

  if (!string_set_tests()) {
    return -1;
  }

  return 0;
}
