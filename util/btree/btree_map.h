#ifndef UTIL_BTREE_MAP_H
#define UTIL_BTREE_MAP_H

#include "util/btree/btree.h"
#include "util/minus.h"

namespace util {
  namespace btree {
    template<typename _Key,
             typename _Tp,
             typename _Compare = util::minus<_Key>,
             size_t _NodeSize = 256>
    class btree_map : public btree<map_parameters<_Key,
                                                  _Tp,
                                                  _Compare,
                                                  _NodeSize> > {
      private:
        typedef map_parameters<_Key, _Tp, _Compare, _NodeSize> parameters_type;
        typedef btree<parameters_type> btree_type;

      public:
        typedef typename btree_type::key_compare key_compare;

        // Constructor.
        btree_map(const key_compare& comp = key_compare());

      private:
        // Disable copy constructor and assignment operator.
        btree_map(const btree_map&) = delete;
        btree_map& operator=(const btree_map&) = delete;
    };

    template<typename _Key,
             typename _Tp,
             typename _Compare = util::minus<_Key>,
             size_t _NodeSize = 256>
    class btree_multimap : public btree<multimap_parameters<_Key,
                                                            _Tp,
                                                            _Compare,
                                                            _NodeSize> > {
      private:
        typedef multimap_parameters<_Key,
                                    _Tp,
                                    _Compare,
                                    _NodeSize> parameters_type;

        typedef btree<parameters_type> btree_type;

      public:
        typedef typename btree_type::key_compare key_compare;

        // Constructor.
        btree_multimap(const key_compare& comp = key_compare());

      private:
        // Disable copy constructor and assignment operator.
        btree_multimap(const btree_multimap&) = delete;
        btree_multimap& operator=(const btree_multimap&) = delete;
    };

    template<typename _Key, typename _Tp, typename _Compare, size_t _NodeSize>
    inline btree_map<_Key,
                     _Tp,
                     _Compare,
                     _NodeSize>::btree_map(const key_compare& comp)
    {
    }

    template<typename _Key, typename _Tp, typename _Compare, size_t _NodeSize>
    inline btree_multimap<_Key,
                          _Tp,
                          _Compare,
                          _NodeSize>::btree_multimap(const key_compare& comp)
    {
    }
  }
}

#endif // UTIL_BTREE_MAP_H
