#ifndef UTIL_BTREE_SET_H
#define UTIL_BTREE_SET_H

#include "util/btree/btree.h"
#include "util/minus.h"

namespace util {
  namespace btree {
    template<typename _Key,
             typename _Compare = util::minus<_Key>,
             size_t _NodeSize = 256>
    class btree_set : public btree<set_parameters<_Key, _Compare, _NodeSize> > {
      private:
        typedef set_parameters<_Key, _Compare, _NodeSize> parameters_type;
        typedef btree<parameters_type> btree_type;

      public:
        typedef typename btree_type::key_compare key_compare;
        typedef typename btree_type::key_type key_type;
        typedef typename btree_type::value_type value_type;

        // Constructor.
        btree_set(const key_compare& comp = key_compare());

        // Insert key.
        bool insert(const key_type& key);

      private:
        // Insert key.
        bool insert(const key_type& key, const value_type& value);

        // Get value.
        bool get(const key_type& key, value_type& value) const = delete;

        // Disable copy constructor and assignment operator.
        btree_set(const btree_set&) = delete;
        btree_set& operator=(const btree_set&) = delete;
    };

    template<typename _Key, typename _Compare, size_t _NodeSize>
    inline btree_set<_Key,
                     _Compare,
                     _NodeSize>::btree_set(const key_compare& comp)
    {
    }

    template<typename _Key, typename _Compare, size_t _NodeSize>
    inline bool btree_set<_Key,
                          _Compare,
                          _NodeSize>::insert(const key_type& key)
    {
      return btree<parameters_type>::insert(key, key);
    }
  }
}

#endif // UTIL_BTREE_SET_H
