#ifndef UTIL_RANDOM_GENERATOR_H
#define UTIL_RANDOM_GENERATOR_H

#include <stdlib.h>
#include <time.h>

namespace util {
  class random_generator {
    public:
      // Constructor.
      random_generator();

      // Destructor.
      ~random_generator();

      // Clear.
      void clear();

      // Initialize.
      bool init(size_t size);

      // Get at.
      bool unordered(size_t i, long& rnd) const;
      bool ordered(size_t i, long& rnd) const;

    private:
      long* _M_unordered;
      long* _M_ordered;
      size_t _M_size;

      // Search.
      bool search(size_t count, long rnd, size_t& pos) const;
  };

  inline random_generator::random_generator()
    : _M_unordered(NULL),
      _M_ordered(NULL),
      _M_size(0)
  {
    srandom(time(NULL));
  }

  inline random_generator::~random_generator()
  {
    clear();
  }

  inline void random_generator::clear()
  {
    if (_M_unordered) {
      free(_M_unordered);
      _M_unordered = NULL;
    }

    if (_M_ordered) {
      free(_M_ordered);
      _M_ordered = NULL;
    }

    _M_size = 0;
  }

  inline bool random_generator::unordered(size_t i, long& rnd) const
  {
    if (i >= _M_size) {
      return false;
    }

    rnd = _M_unordered[i];

    return true;
  }

  inline bool random_generator::ordered(size_t i, long& rnd) const
  {
    if (i >= _M_size) {
      return false;
    }

    rnd = _M_ordered[i];

    return true;
  }
}

#endif // UTIL_RANDOM_GENERATOR_H
