#include <string.h>
#include "util/random_generator.h"

bool util::random_generator::init(size_t size)
{
  if ((_M_unordered = reinterpret_cast<long*>(
                        malloc(size * sizeof(long))
                      )) == NULL) {
    return false;
  }

  if ((_M_ordered = reinterpret_cast<long*>(
                      malloc(size * sizeof(long))
                    )) == NULL) {
    return false;
  }

  for (size_t i = 0; i < size; i++) {
    long rnd;
    size_t pos;
    do {
      rnd = random();
    } while (search(i, rnd, pos));

    if (pos < i) {
      memmove(&_M_ordered[pos + 1],
              &_M_ordered[pos],
              (i - pos) * sizeof(long));
    }

    _M_unordered[i] = rnd;
    _M_ordered[pos] = rnd;
  }

  _M_size = size;

  return true;
}

bool util::random_generator::search(size_t count, long rnd, size_t& pos) const
{
  int left = 0;
  int right = count - 1;

  while (left <= right) {
    int mid = (left + right) / 2;

    if (rnd < _M_ordered[mid]) {
      right = mid - 1;
    } else if (rnd > _M_ordered[mid]) {
      left = mid + 1;
    } else {
      pos = mid;
      return true;
    }
  }

  pos = left;

  return false;
}
