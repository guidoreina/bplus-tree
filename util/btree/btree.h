#ifndef UTIL_BTREE_H
#define UTIL_BTREE_H

#include <stdint.h>
#include <memory>
#include "util/move.h"

namespace util {
  namespace btree {
    // Common B-tree parameters.
    template<typename _Key, typename _Compare, size_t _NodeSize>
    struct common_parameters {
      typedef _Key key_type;
      typedef _Compare key_compare;

      typedef struct {
        uint32_t type:1;
        uint32_t count:31;
      } node_header;

      static const size_t kNodeSize = _NodeSize;
    };

    // Set parameters.
    template<typename _Key, typename _Compare, size_t _NodeSize>
    struct set_parameters
      : public common_parameters<_Key, _Compare, _NodeSize> {
      typedef _Key value_type;

      static const size_t kValueSize = 0;

      static const bool kDuplicates = false;

      // Internal nodes don't have data.
      //
      // kNodeSize >= sizeof(node_header) +
      //              (kInternalNodeMaxKeys * sizeof(_Key)) +
      //              ((kInternalNodeMaxKeys + 1) * sizeof(void*))
      //
      // kNodeSize - sizeof(node_header) >=
      //                         (kInternalNodeMaxKeys * sizeof(_Key)) +
      //                         ((kInternalNodeMaxKeys + 1) * sizeof(void*))
      //
      // kNodeSize - sizeof(node_header) >=
      //                         kInternalNodeMaxKeys * sizeof(_Key) +
      //                         kInternalNodeMaxKeys * sizeof(void*) +
      //                         sizeof(void*)
      //
      // kNodeSize - sizeof(node_header) - sizeof(void*) >=
      //                         kInternalNodeMaxKeys * sizeof(_Key) +
      //                         kInternalNodeMaxKeys * sizeof(void*)
      //
      // kNodeSize - sizeof(node_header) - sizeof(void*) >=
      //                 kInternalNodeMaxKeys * (sizeof(_Key) + sizeof(void*))
      //
      // kNodeSize - sizeof(node_header) - sizeof(void*)
      // ----------------------------------------------- >= kInternalNodeMaxKeys
      //           sizeof(_Key) + sizeof(void*)

      static const size_t kInternalNodeMaxKeys =
           (common_parameters<_Key, _Compare, _NodeSize>::kNodeSize -
            sizeof(typename common_parameters<_Key,
                                              _Compare,
                                              _NodeSize>::node_header) -
            sizeof(void*)) /
           (sizeof(_Key) + sizeof(void*));

      // Leaf nodes need two pointers, one to point to the previous node and
      // the other one to point to the next node.
      //
      // kNodeSize >= sizeof(node_header) +
      //              (kLeafNodeMaxKeys * sizeof(_Key)) +
      //              (2 * sizeof(void*))
      //
      // kNodeSize - sizeof(node_header) - (2 * sizeof(void*)) >=
      //                                 kLeafNodeMaxKeys * sizeof(_Key)
      //
      // kNodeSize - sizeof(node_header) - (2 * sizeof(void*))
      // ----------------------------------------------------- >= kLeafNodeMaxKeys
      //                      sizeof(_Key)

      static const size_t kLeafNodeMaxKeys =
           (common_parameters<_Key, _Compare, _NodeSize>::kNodeSize -
            sizeof(typename common_parameters<_Key,
                                              _Compare,
                                              _NodeSize>::node_header) -
            (2 * sizeof(void*))) /
           sizeof(_Key);
    };

    // Common map parameters.
    template<typename _Key, typename _Tp, typename _Compare, size_t _NodeSize>
    struct common_map_parameters
      : public common_parameters<_Key, _Compare, _NodeSize> {
      typedef _Tp value_type;

      static const size_t kValueSize = sizeof(_Tp);

      // Internal nodes don't have data.
      //
      // _NodeSize >= sizeof(node_header) +
      //              (kInternalNodeMaxKeys * sizeof(_Key)) +
      //              ((kInternalNodeMaxKeys + 1) * sizeof(void*))
      //
      // _NodeSize - sizeof(node_header) >=
      //                         (kInternalNodeMaxKeys * sizeof(_Key)) +
      //                         ((kInternalNodeMaxKeys + 1) * sizeof(void*))
      //
      // _NodeSize - sizeof(node_header) >=
      //                         kInternalNodeMaxKeys * sizeof(_Key) +
      //                         kInternalNodeMaxKeys * sizeof(void*) +
      //                         sizeof(void*)
      //
      // _NodeSize - sizeof(node_header) - sizeof(void*) >=
      //                         kInternalNodeMaxKeys * sizeof(_Key) +
      //                         kInternalNodeMaxKeys * sizeof(void*)
      //
      // _NodeSize - sizeof(node_header) - sizeof(void*) >=
      //                 kInternalNodeMaxKeys * (sizeof(_Key) + sizeof(void*))
      //
      // _NodeSize - sizeof(node_header) - sizeof(void*)
      // ----------------------------------------------- >= kInternalNodeMaxKeys
      //           sizeof(_Key) + sizeof(void*)

      static const size_t kInternalNodeMaxKeys =
           (common_parameters<_Key, _Compare, _NodeSize>::kNodeSize -
            sizeof(typename common_parameters<_Key,
                                              _Compare,
                                              _NodeSize>::node_header) -
            sizeof(void*)) /
           (sizeof(_Key) + sizeof(void*));

      // Leaf nodes need two pointers, one to point to the previous node and
      // the other one to point to the next node.
      //
      // _NodeSize >= sizeof(node_header) +
      //              (kLeafNodeMaxKeys * sizeof(_Key)) +
      //              (kLeafNodeMaxKeys * sizeof(_Tp)) +
      //              (2 * sizeof(void*))
      //
      // _NodeSize - sizeof(node_header) - (2 * sizeof(void*)) >=
      //                                 (kLeafNodeMaxKeys * sizeof(_Key)) +
      //                                 (kLeafNodeMaxKeys * sizeof(_Tp))
      //
      // _NodeSize - sizeof(node_header) - (2 * sizeof(void*)) >=
      //                  kLeafNodeMaxKeys * (sizeof(_Key) + sizeof(_Tp))
      //
      // _NodeSize - sizeof(node_header) - (2 * sizeof(void*))
      // ----------------------------------------------------- >= kLeafNodeMaxKeys
      //               sizeof(_Key) + sizeof(_Tp)

      static const size_t kLeafNodeMaxKeys =
           (common_parameters<_Key, _Compare, _NodeSize>::kNodeSize -
            sizeof(typename common_parameters<_Key,
                                              _Compare,
                                              _NodeSize>::node_header) -
            (2 * sizeof(void*))) /
           (sizeof(_Key) + sizeof(_Tp));
    };

    // Map parameters.
    template<typename _Key, typename _Tp, typename _Compare, size_t _NodeSize>
    struct map_parameters
      : public common_map_parameters<_Key, _Tp, _Compare, _NodeSize> {
      static const bool kDuplicates = false;
    };

    // Multimap parameters.
    template<typename _Key, typename _Tp, typename _Compare, size_t _NodeSize>
    struct multimap_parameters
      : public common_map_parameters<_Key, _Tp, _Compare, _NodeSize> {
      static const bool kDuplicates = true;
    };

    template<typename _Parameters>
    class btree {
      private:
        class node {
          friend class btree;

          public:
            typedef typename btree::parameters_type parameters_type;
            typedef typename btree::key_type key_type;
            typedef typename btree::value_type value_type;
            typedef typename btree::key_compare key_compare;

            enum type {
              kInternal,
              kLeaf
            };

            // Constructor.
            node(uint8_t* data);

            // Destructor.
            ~node();

            // Create node.
            static node* create(type type);

            // Node full?
            bool full() const;

            // Minimum number of keys?
            bool minkeys() const;

            // Find.
            bool find(const key_type& key,
                      const key_compare& comp,
                      uint16_t& pos) const;

            // Lower bound.
            bool lower_bound(const key_type& key,
                             const key_compare& comp,
                             uint16_t& pos) const;

            // Upper bound.
            bool upper_bound(const key_type& key,
                             const key_compare& comp,
                             uint16_t& pos) const;

            // Insert key in non-full node.
            static bool insert_non_full(node* x,
                                        const key_type& key,
                                        const value_type& value,
                                        const key_compare& comp,
                                        size_t& nkeys);

            // Split child.
            bool split_child(uint16_t i);

            // Erase key.
            static bool erase(node*& root,
                              const key_type& key,
                              const key_compare& comp);

            // Get previous.
            const node* prev() const;
            node* prev();

            // Set previous node.
            void prev(node* n);

            // Get next node.
            const node* next() const;
            node* next();

            // Set next node.
            void next(node* n);

          protected:
            static const size_t kValueSize = parameters_type::kValueSize;

            static const size_t kInternalNodeMaxKeys =
                                (parameters_type::kInternalNodeMaxKeys >= 3) ?
                                       parameters_type::kInternalNodeMaxKeys :
                                       3;

            static const size_t kInternalNodeMinKeys =
                                ((kInternalNodeMaxKeys + 1) / 2) - 1;

            static const size_t kInternalNodeMedian = kInternalNodeMaxKeys >> 1;

            static const size_t kInternalNodeSize =
                                sizeof(typename parameters_type::node_header) +
                                (kInternalNodeMaxKeys * sizeof(key_type)) +
                                ((kInternalNodeMaxKeys + 1) * sizeof(node*));

            static const size_t kLeafNodeMaxKeys =
                                (parameters_type::kLeafNodeMaxKeys >= 3) ?
                                       parameters_type::kLeafNodeMaxKeys :
                                       3;

            static const size_t kLeafNodeMinKeys = kLeafNodeMaxKeys >> 1;

            static const size_t kLeafNodeMedian =
                                (kLeafNodeMaxKeys + 1) >> 1;

            static const size_t kLeafNodeSize =
                                sizeof(typename parameters_type::node_header) +
                                (kLeafNodeMaxKeys * sizeof(key_type)) +
                                (kLeafNodeMaxKeys * kValueSize) +
                                (2 * sizeof(node*));

            static const bool kDuplicates = parameters_type::kDuplicates;

            uint8_t* _M_data;

            typename parameters_type::node_header* _M_header;
            key_type* _M_keys;

            // For internal nodes.
            node** _M_children;

            // For leaf nodes.
            value_type* _M_values;

            node** _M_prev;
            node** _M_next;

            // Rebalance left to right.
            static void rebalance_left_to_right(node* x, uint16_t i);

            // Rebalance right to left.
            static void rebalance_right_to_left(node* x, uint16_t i);

            // Merge.
            static void merge(node* x, node* y, node* z, uint16_t i);

            enum operation_result {
              kNoop,
              kRebalancedLeftToRight,
              kRebalancedRightToLeft,
              kMerged,
              kShrinked
            };

            // Try to rebalance or merge subtree.
            static operation_result try_rebalance_or_merge_subtree(node* x,
                                                                   node*& root,
                                                                   uint16_t i);

            // Try to rebalance or merge.
            static operation_result try_rebalance_or_merge(node* x,
                                                           node*& root,
                                                           uint16_t& i);

            // Disable copy constructor and assignment operator.
            node(const node&) = delete;
            node& operator=(const node&) = delete;
        };

      public:
        typedef _Parameters parameters_type;
        typedef typename _Parameters::key_type key_type;
        typedef typename _Parameters::value_type value_type;
        typedef typename _Parameters::key_compare key_compare;

        class iterator {
          friend class btree;

          public:
            typedef typename btree::key_type key_type;
            typedef typename btree::value_type value_type;

            // Get key.
            const key_type& key() const;

            // Get value.
            value_type& value();

            // Comparison operators.
            bool operator==(const iterator& other) const;
            bool operator!=(const iterator& other) const;

          private:
            node* _M_node;
            uint16_t _M_pos;
        };

        class const_iterator {
          friend class btree;

          public:
            typedef typename btree::key_type key_type;
            typedef typename btree::value_type value_type;

            // Get key.
            const key_type& key() const;

            // Get value.
            const value_type& value() const;

            // Comparison operators.
            bool operator==(const const_iterator& other) const;
            bool operator!=(const const_iterator& other) const;

          private:
            const node* _M_node;
            uint16_t _M_pos;
        };

        // Constructor.
        btree(const key_compare& comp = key_compare());

        // Destructor.
        ~btree();

        // Clear.
        void clear();

        // Get number of keys.
        size_t count() const;

        // Insert key.
        bool insert(const key_type& key, const value_type& value);

        // Erase key.
        bool erase(const key_type& key);

        // Get value.
        bool get(const key_type& key, value_type& value) const;

        // Begin.
        bool begin(iterator& it);
        bool begin(const_iterator& it) const;

        // End.
        bool end(iterator& it);
        bool end(const_iterator& it) const;

        // Previous.
        bool prev(iterator& it);
        bool prev(const_iterator& it) const;

        // Next.
        bool next(iterator& it);
        bool next(const_iterator& it) const;

        // Find.
        bool find(const key_type& key, iterator& it);
        bool find(const key_type& key, const_iterator& it) const;

        // Lower bound.
        bool lower_bound(const key_type& key, iterator& it);
        bool lower_bound(const key_type& key, const_iterator& it) const;

        // Upper bound.
        bool upper_bound(const key_type& key, iterator& it);
        bool upper_bound(const key_type& key, const_iterator& it) const;

        // Equal range.
        bool equal_range(const key_type& key, iterator& begin, iterator& end);
        bool equal_range(const key_type& key,
                         const_iterator& begin,
                         const_iterator& end) const;

      private:
        static const bool kDuplicates = parameters_type::kDuplicates;

        key_compare _M_comp;

        node* _M_root;
        size_t _M_nkeys;

        // Disable copy constructor and assignment operator.
        btree(const btree&) = delete;
        btree& operator=(const btree&) = delete;
    };

    template<typename _Parameters>
    inline btree<_Parameters>::node::node(uint8_t* data)
      : _M_data(data)
    {
    }

    template<typename _Parameters>
    inline btree<_Parameters>::node::~node()
    {
      if (_M_data) {
        // Invoke the destructors.
        uint16_t count = _M_header->count;
        for (uint16_t i = 0; i < count; i++) {
          _M_keys[i].key_type::~key_type();
        }

        // Internal node?
        if (_M_header->type == kInternal) {
          for (uint16_t i = 0; i <= count; i++) {
            delete _M_children[i];
          }
        } else if (kValueSize > 0) {
          for (uint16_t i = 0; i < count; i++) {
            _M_values[i].value_type::~value_type();
          }
        }

        free(_M_data);
      }
    }

    template<typename _Parameters>
    inline typename btree<_Parameters>::node*
    btree<_Parameters>::node::create(type type)
    {
      size_t node_size;
      if (type == kInternal) {
        node_size = kInternalNodeSize;
      } else {
        node_size = kLeafNodeSize;
      }

      uint8_t* data;
      if ((data = reinterpret_cast<uint8_t*>(calloc(node_size, 1))) == NULL) {
        return NULL;
      }

      node* n;
      if ((n = new (std::nothrow) node(data)) == NULL) {
        free(data);
        return NULL;
      }

      // Initialize header.
      n->_M_header = reinterpret_cast<typename parameters_type::node_header*>(
                       data
                     );

      n->_M_header->type = type;

      // Initialize pointer to keys.
      data += sizeof(typename parameters_type::node_header);
      n->_M_keys = reinterpret_cast<key_type*>(data);
      key_type* keys = n->_M_keys;

      // Internal node?
      if (type == kInternal) {
        // Invoke constructors.
        for (uint16_t i = 0; i < kInternalNodeMaxKeys; i++) {
          new (&keys[i]) key_type();
        }

        // Initialize pointer to childen nodes.
        data += (kInternalNodeMaxKeys * sizeof(key_type));
        n->_M_children = reinterpret_cast<node**>(data);
      } else {
        // Initialize pointer to values.
        data += (kLeafNodeMaxKeys * sizeof(key_type));

        if (kValueSize > 0) {
          n->_M_values = reinterpret_cast<value_type*>(data);

          // Invoke constructors.
          value_type* values = n->_M_values;
          for (uint16_t i = 0; i < kLeafNodeMaxKeys; i++) {
            new (&keys[i]) key_type();
            new (&values[i]) value_type();
          }
        } else {
          n->_M_values = reinterpret_cast<value_type*>(keys);

          // Invoke constructors.
          for (uint16_t i = 0; i < kLeafNodeMaxKeys; i++) {
            new (&keys[i]) key_type();
          }
        }

        // Initialize pointer to previous node.
        data += (kLeafNodeMaxKeys * node::kValueSize);
        n->_M_prev = reinterpret_cast<node**>(data);

        // Initialize pointer to next node.
        data += sizeof(node*);
        n->_M_next = reinterpret_cast<node**>(data);
      }

      return n;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::node::full() const
    {
      return (_M_header->type == kInternal) ?
                              (_M_header->count == kInternalNodeMaxKeys) :
                              (_M_header->count == kLeafNodeMaxKeys);
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::node::minkeys() const
    {
      return (_M_header->type == kInternal) ?
                              (_M_header->count == kInternalNodeMinKeys) :
                              (_M_header->count == kLeafNodeMinKeys);
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    // Function: find                                                         //
    // Description: returns the position of the key if found; the position of //
    //              the first element which is greater than key otherwise.    //
    //                                                                        //
    // Parameters:                                                            //
    //   - [in] key: key to be searched.                                      //
    //   - [in] comp: compare function.                                       //
    //   - [out] pos: position of the key if found; the position of the first //
    //                element which is greater than key otherwise.            //
    //                                                                        //
    // Returns: true: key was found; false otherwise.                         //
    //                                                                        //
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    template<typename _Parameters>
    bool btree<_Parameters>::node::find(const key_type& key,
                                        const key_compare& comp,
                                        uint16_t& pos) const
    {
      // Pre-condition: _M_keys is sorted.

      int left = 0;
      int right = _M_header->count - 1;

      while (left <= right) {
        // Loop invariant:
        // {(_M_keys[left - 1] < key) && (_M_keys[right + 1] > key)}

        int mid = (left + right) / 2;

        int r;
        if ((r = comp(_M_keys[mid], key)) < 0) {
          // _M_keys[mid] < key
          left = mid + 1;
          // _M_keys[left - 1] < key
        } else if (r > 0) {
          // _M_keys[mid] > key
          right = mid - 1;
          // _M_keys[right + 1] > key
        } else {
          // _M_keys[mid] == key
          pos = mid;
          return true;
        }
      }

      pos = left;

      // Post-condition:
      // (left > right) &&
      // (_M_keys[left - 1] < key) &&
      // (_M_keys[right + 1] > key)

      return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    // Function: lower_bound                                                  //
    // Description: returns the position of the first element which is        //
    //              greater or equal than key.                                //
    //                                                                        //
    // Parameters:                                                            //
    //   - [in] key: key to be searched.                                      //
    //   - [in] comp: compare function.                                       //
    //   - [out] pos: position of the first element which is greater or equal //
    //                than key.                                               //
    //                                                                        //
    // Returns: true: key was found; false otherwise.                         //
    //                                                                        //
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    template<typename _Parameters>
    bool btree<_Parameters>::node::lower_bound(const key_type& key,
                                               const key_compare& comp,
                                               uint16_t& pos) const
    {
      // Pre-condition: _M_keys is sorted.

      int left = 0;
      int right = _M_header->count;

      bool ret = false;

      while (left != right) {
        // Loop invariant:
        // {(_M_keys[left - 1] < key) && (_M_keys[right] >= key)}

        int mid = (left + right) / 2;

        int r;
        if ((r = comp(_M_keys[mid], key)) < 0) {
          // _M_keys[mid] < key
          left = mid + 1;
          // _M_keys[left - 1] < key
        } else if (r > 0) {
          // _M_keys[mid] > key
          right = mid;
          // _M_keys[right] >= key
        } else {
          // _M_keys[mid] == key
          right = mid;
          ret = true;
          // _M_keys[right] >= key
        }
      }

      pos = left;

      // Post-condition:
      // (left == right) && (_M_keys[left - 1] < key) && (_M_keys[right] >= key)

      return ret;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    //                                                                        //
    // Function: upper_bound                                                  //
    // Description: returns the position of the first element which is        //
    //              greater than key.                                         //
    //                                                                        //
    // Parameters:                                                            //
    //   - [in] key: key to be searched.                                      //
    //   - [in] comp: compare function.                                       //
    //   - [out] pos: position of the first element which is greater than     //
    //                key.                                                    //
    //                                                                        //
    // Returns: true: key was found; false otherwise.                         //
    //                                                                        //
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    template<typename _Parameters>
    bool btree<_Parameters>::node::upper_bound(const key_type& key,
                                               const key_compare& comp,
                                               uint16_t& pos) const
    {
      // Pre-condition: _M_keys is sorted.

      int left = 0;
      int right = _M_header->count;

      bool ret = false;

      while (left != right) {
        // Loop invariant:
        // {(_M_keys[left - 1] <= key) && (_M_keys[right] > key)}

        int mid = (left + right) / 2;

        int r;
        if ((r = comp(_M_keys[mid], key)) < 0) {
          // _M_keys[mid] < key
          left = mid + 1;
          // _M_keys[left - 1] <= key
        } else if (r > 0) {
          // _M_keys[mid] > key
          right = mid;
          // _M_keys[right] > key
        } else {
          // _M_keys[mid] == key
          left = mid + 1;
          ret = true;
          // _M_keys[left - 1] <= key
        }
      }

      pos = left;

      // Post-condition:
      // (left == right) && (_M_keys[left - 1] <= key) && (_M_keys[right] > key)

      return ret;
    }

    template<typename _Parameters>
    bool btree<_Parameters>::node::insert_non_full(node* x,
                                                   const key_type& key,
                                                   const value_type& value,
                                                   const key_compare& comp,
                                                   size_t& nkeys)
    {
      // While 'x' is an internal node...
      while (x->_M_header->type == kInternal) {
        uint16_t i;
        x->upper_bound(key, comp, i);

        // If the child is full...
        if (x->_M_children[i]->full()) {
          if (!x->split_child(i)) {
            return false;
          }

          if (comp(x->_M_keys[i], key) <= 0) {
            i++;
          }
        }

        x = x->_M_children[i];
      }

      // Leaf node.
      // Pre-condition: _M_keys is sorted.

      // If the key has been already inserted and duplicates are not allowed...
      uint16_t i;
      if ((x->upper_bound(key, comp, i)) && (!kDuplicates)) {
        // If the tree might have values...
        if (kValueSize > 0) {
          // Update value.
          x->_M_values[i] = value;
        }

        return true;
      }

      key_type* keys = x->_M_keys;

      // If the tree might have values...
      if (kValueSize > 0) {
        // Move bigger keys with their values one position to the right.
        value_type* values = x->_M_values;
        for (uint16_t j = x->_M_header->count; j > i; j--) {
          keys[j] = util::move(keys[j - 1]);
          values[j] = util::move(values[j - 1]);
        }

        // Save value.
        values[i] = value;
      } else {
        // Move bigger keys one position to the right.
        for (uint16_t j = x->_M_header->count; j > i; j--) {
          keys[j] = util::move(keys[j - 1]);
        }
      }

      // Save key.
      keys[i] = key;

      // Increment number of elements.
      x->_M_header->count++;

      // Increment number of keys.
      nkeys++;

      return true;
    }

    template<typename _Parameters>
    bool btree<_Parameters>::node::split_child(uint16_t i)
    {
      node* y = _M_children[i];

      // Create child node.
      node* z;
      if ((z = create(static_cast<type>(y->_M_header->type))) == NULL) {
        return false;
      }

      key_type* ykeys = y->_M_keys;
      key_type* zkeys = z->_M_keys;

      uint16_t median;
      uint16_t ycount;
      uint16_t zcount;

      // If 'y' is an internal node...
      if (y->_M_header->type == kInternal) {
        // The median key of 'y' moves up into its parent.
        // The median is calculated as: median = floor(kMaxKeys / 2).

        // If 'kMaxKeys' is even, for example: 4:
        // median = floor(4 / 2) = 2
        //
        // - Before the split:
        //
        // Index:    0       1       2       3
        //       +-------+-------+-------+-------+
        //       |  100  |  200  |  300  |  400  |
        //   +---+---+---+---+---+---+---+---+---+---+
        //   |       |       |       |       |       |
        //   +-------+-------+-------+-------+-------+
        //       |       |       |       |       |
        //      <100    <200    <300    <400   >=400
        //
        //
        // - After the split:
        //   'median' keys (2 keys) stay in left child; the rest, except the
        //   median key (1 key), is moved to the right child.
        //
        // Index:    0       1                  0
        //       +-------+-------+          +-------+
        //       |  100  |  200  |          |  400  |
        //   +---+---+---+---+---+---+  +---+---+---+---+
        //   |       |       |       |  |       |       |
        //   +-------+-------+-------+  +-------+-------+
        //       |       |       |          |       |
        //      <100    <200    <300       <400   >=400
        //
        // Median key (300) is moved up into its parent.
        //
        //
        //
        // If 'kMaxKeys' is odd, for example: 5:
        // median = floor(5 / 2) = 2
        //
        // - Before the split:
        //
        // Index:    0       1       2       3       4
        //       +-------+-------+-------+-------+-------+
        //       |  100  |  200  |  300  |  400  |  500  |
        //   +---+---+---+---+---+---+---+---+---+---+---+---+
        //   |       |       |       |       |       |       |
        //   +-------+-------+-------+-------+-------+-------+
        //       |       |       |       |       |       |
        //      <100    <200    <300    <400    <500   >=500
        //
        //
        // - After the split:
        //   'median' keys (2 keys) stay in left child; the rest, except the
        //   median key (2 keys), is moved to the right child.
        //
        // Index:    0       1                  0       1
        //       +-------+-------+          +-------+-------+
        //       |  100  |  200  |          |  400  |  500  |
        //   +---+---+---+---+---+---+  +---+---+---+---+---+---+
        //   |       |       |       |  |       |       |       |
        //   +-------+-------+-------+  +-------+-------+-------+
        //       |       |       |          |       |       |
        //      <100    <200    <300       <400    <500   >=500
        //
        // Median key (300) is moved up into its parent.
        //

        // median = floor(kMaxKeys / 2).
        median = kInternalNodeMedian;

        ycount = median;
        zcount = kInternalNodeMaxKeys - ycount - 1;

        node** ychildren = y->_M_children;
        node** zchildren = z->_M_children;

        // Copy keys and pointers from node 'y' to node 'z'.
        for (uint16_t j = 0; j < zcount; j++) {
          zkeys[j] = util::move(ykeys[ycount + 1 + j]);
          zchildren[j] = ychildren[ycount + 1 + j];
        }

        zchildren[zcount] = ychildren[kInternalNodeMaxKeys];
      } else {
        // The first key of 'z' is copied into its parent.
        // The median is calculated as: median = ceiling(kMaxKeys / 2).

        // If 'kMaxKeys' is even, for example: 4:
        // median = ceiling(4 / 2) = 2
        //
        // - Before the split:
        //
        // Index:    0       1       2       3
        //       +-------+-------+-------+-------+
        //       |  100  |  200  |  300  |  400  |
        //       +-------+-------+-------+-------+-------+
        //       |   a   |   b   |   c   |   d   |    ---|--->
        //       +-------+-------+-------+-------+-------+
        //
        //
        // - After the split:
        //   'median' keys (2 keys) stay in left child; the rest (2 keys)
        //   is moved to the right child.
        //
        // Index:    0       1                    0       1
        //       +-------+-------+            +-------+-------+
        //       |  100  |  200  |            |  300  |  400  |
        //       +-------+-------+-------+    +-------+-------+-------+
        //       |   a   |   b   |    ---|--->|   c   |   d   |    ---|--->
        //       +-------+-------+-------+    +-------+-------+-------+
        //
        // The first key of 'z' (300) is copied into its parent.
        //
        //
        //
        // If 'kMaxKeys' is odd, for example: 5:
        // median = ceiling(5 / 2) = 3
        //
        // - Before the split:
        //
        // Index:    0       1       2       3       4
        //       +-------+-------+-------+-------+-------+
        //       |  100  |  200  |  300  |  400  |  500  |
        //       +-------+-------+-------+-------+-------+-------+
        //       |   a   |   b   |   c   |   d   |   e   |    ---|--->
        //       +-------+-------+-------+-------+-------+-------+
        //
        //
        // - After the split:
        //   'median' keys (3 keys) stay in left child; the rest (2 keys)
        //   is moved to the right child.
        //
        // Index:    0       1       2                    0       1
        //       +-------+-------+-------+            +-------+-------+
        //       |  100  |  200  |  300  |            |  400  |  500  |
        //       +-------+-------+-------+-------+    +-------+-------+-------+
        //       |   a   |   b   |   c   |    ---|--->|   d   |   e   |    ---|--->
        //       +-------+-------+-------+-------+    +-------+-------+-------+
        //
        // The first key of 'z' (400) is copied into its parent.
        //

        // median = ceiling(kMaxKeys / 2).
        ycount = kLeafNodeMedian;
        zcount = kLeafNodeMaxKeys - ycount;

        // If the tree might have values...
        if (kValueSize > 0) {
          value_type* yvalues = y->_M_values;
          value_type* zvalues = z->_M_values;

          // Copy keys and values from node 'y' to node 'z'.
          for (uint16_t j = 0; j < zcount; j++) {
            zkeys[j] = util::move(ykeys[ycount + j]);
            zvalues[j] = util::move(yvalues[ycount + j]);
          }
        } else {
          // Copy keys from node 'y' to node 'z'.
          for (uint16_t j = 0; j < zcount; j++) {
            zkeys[j] = util::move(ykeys[ycount + j]);
          }
        }

        z->prev(y);
        z->next(y->next());

        if (z->next()) {
          z->next()->prev(z);
        }

        y->next(z);
      }

      y->_M_header->count = ycount;
      z->_M_header->count = zcount;

      // If i = 2, median key = 300 and the node 'x' looks like:
      //
      // Index:    0       1       2       3
      //       +-------+-------+-------+-------+
      //       |  100  |  200  |  400  |  500  |
      //   +---+---+---+---+---+---+---+---+---+---+
      //   |       |       |       |       |       |
      //   +-------+-------+-------+-------+-------+
      //       |       |       |       |       |
      //      <100    <200    <400    <500   >=500
      // Node:                 y
      //
      // after adding the median key 300 and the node 'z':
      //
      // Index:    0       1       2       3       4
      //       +-------+-------+-------+-------+-------+
      //       |  100  |  200  |  300  |  400  |  500  |
      //   +---+---+---+---+---+---+---+---+---+---+---+---+
      //   |       |       |       |       |       |       |
      //   +-------+-------+-------+-------+-------+-------+
      //       |       |       |       |       |       |
      //      <100    <200    <300    <400    <500   >=500
      // Node:                 y       z
      //

      // Shift keys and pointers one position to the right.
      for (uint16_t j = _M_header->count; j > i; j--) {
        _M_keys[j] = util::move(_M_keys[j - 1]);
        _M_children[j + 1] = _M_children[j];
      }

      _M_children[i + 1] = z;

      // If 'y' is an internal node...
      if (y->_M_header->type == kInternal) {
        _M_keys[i] = util::move(y->_M_keys[median]);
      } else {
        _M_keys[i] = z->_M_keys[0];
      }

      _M_header->count++;

      return true;
    }

    template<typename _Parameters>
    bool btree<_Parameters>::node::erase(node*& root,
                                         const key_type& key,
                                         const key_compare& comp)
    {
      bool search_in_next_node = false;

      // While 'x' is an internal node...
      node* x = root;
      while (x->_M_header->type == kInternal) {
        uint16_t i;
        if (x->lower_bound(key, comp, i)) {
          if (!kDuplicates) {
            i++;
          } else {
            switch (try_rebalance_or_merge_subtree(x, root, i + 1)) {
              case kRebalancedLeftToRight:
                if (comp(key, x->_M_keys[i]) > 0) {
                  i++;
                  search_in_next_node = false;
                } else {
                  search_in_next_node = true;
                }

                break;
              case kShrinked:
                x = root;
                search_in_next_node = false;

                continue;
              default:
                search_in_next_node = true;
            }
          }
        }

        if (try_rebalance_or_merge(x, root, i) == kShrinked) {
          x = root;
          search_in_next_node = false;

          continue;
        }

        x = x->_M_children[i];
      }

      // Leaf node.

      // Search key in leaf node.
      uint16_t i;
      if (!x->lower_bound(key, comp, i)) {
        if ((!kDuplicates) || (!search_in_next_node)) {
          // Key not found.
          return false;
        }

        x = x->next();

        if (comp(key, x->_M_keys[0]) != 0) {
          // Key not found.
          return false;
        }

        i = 0;
      }

      key_type* keys = x->_M_keys;

      // Invoke key's destructor.
      keys[i].key_type::~key_type();

      // Invoke key's constructor.
      new (&keys[i]) key_type();

      uint16_t count = x->_M_header->count;
      if (kValueSize > 0) {
        value_type* values = x->_M_values;

        // Invoke value's destructor.
        values[i].value_type::~value_type();

        // Invoke value's constructor.
        new (&values[i]) value_type();

        // Shift keys and values one position to the left.
        for (++i; i < count; i++) {
          keys[i - 1] = util::move(keys[i]);
          values[i - 1] = util::move(values[i]);
        }
      } else {
        // Shift keys one position to the left.
        for (++i; i < count; i++) {
          keys[i - 1] = util::move(keys[i]);
        }
      }

      // Decrement number of elements.
      x->_M_header->count--;

      return true;
    }

    template<typename _Parameters>
    inline const typename btree<_Parameters>::node*
    btree<_Parameters>::node::prev() const
    {
      return *_M_prev;
    }

    template<typename _Parameters>
    inline typename btree<_Parameters>::node* btree<_Parameters>::node::prev()
    {
      return *_M_prev;
    }

    template<typename _Parameters>
    inline void btree<_Parameters>::node::prev(node* n)
    {
      *_M_prev = n;
    }

    template<typename _Parameters>
    inline const typename btree<_Parameters>::node*
    btree<_Parameters>::node::next() const
    {
      return *_M_next;
    }

    template<typename _Parameters>
    inline typename btree<_Parameters>::node* btree<_Parameters>::node::next()
    {
      return *_M_next;
    }

    template<typename _Parameters>
    inline void btree<_Parameters>::node::next(node* n)
    {
      *_M_next = n;
    }

    template<typename _Parameters>
    void btree<_Parameters>::node::rebalance_left_to_right(node* x, uint16_t i)
    {
      node* y = x->_M_children[--i]; // Left sibling.
      node* z = x->_M_children[i + 1];

      uint16_t ycount = y->_M_header->count;

      key_type* xkeys = x->_M_keys;
      key_type* ykeys = y->_M_keys;
      key_type* zkeys = z->_M_keys;

      // If 'z' is an internal node...
      if (z->_M_header->type == kInternal) {
        // - Before the rebalance:
        //
        // Index:                           0
        //                              +-------+
        //                              |  400  |
        //                          +---+---+---+---+
        //                          |       |       |
        //                          +-------+-------+
        //                              |       |
        //                   +----------+       +----------+
        //                   |                             |
        //                   v                             v
        //
        // Index:    0       1       2                     0
        //       +-------+-------+-------+             +-------+
        //       |  100  |  200  |  300  |             |  500  |
        //   +---+---+---+---+---+---+---+---+     +---+---+---+---+
        //   |       |       |       |       |     |       |       |
        //   +-------+-------+-------+-------+     +-------+-------+
        //       |       |       |       |             |       |
        //      <100    <200    <300    <400          <500   >=500
        //
        //
        // - After the rebalance:
        //   Parent's key (400) is moved down into the right sibling.
        //   Left sibling's rightmost key (300) is moved up into its parent.
        //   Left sibling's rightmost child pointer is moved into the
        //   right sibling.
        //
        // Index:                           0
        //                              +-------+
        //                              |  300  |
        //                          +---+---+---+---+
        //                          |       |       |
        //                          +-------+-------+
        //                              |       |
        //                   +----------+       +----------+
        //                   |                             |
        //                   v                             v
        //
        // Index:        0       1                     0       1
        //           +-------+-------+             +-------+-------+
        //           |  100  |  200  |             |  400  |  500  |
        //       +---+---+---+---+---+---+     +---+---+---+---+---+---+
        //       |       |       |       |     |       |       |       |
        //       +-------+-------+-------+     +-------+-------+-------+
        //           |       |       |             |       |       |
        //          <100    <200    <300          <400    <500   >=500
        //

        node** zchildren = z->_M_children;

        // Shift keys and pointers one position to the right.
        for (uint16_t j = z->_M_header->count; j > 0; j--) {
          zkeys[j] = util::move(zkeys[j - 1]);
          zchildren[j + 1] = zchildren[j];
        }

        zchildren[1] = zchildren[0];

        // Move key from 'x' down into 'z'.
        zkeys[0] = util::move(xkeys[i]);

        // Move rightmost key from left sibling up into 'x'.
        xkeys[i] = util::move(ykeys[ycount - 1]);

        // Move rightmost child pointer from left sibling into 'z'.
        zchildren[0] = y->_M_children[ycount];
      } else {
        // 'z' is a leaf node.

        // - Before the rebalance:
        //
        // Index:                           0
        //                              +-------+
        //                              |  400  |
        //                          +---+---+---+---+
        //                          |       |       |
        //                          +-------+-------+
        //                              |       |
        //                   +----------+       +----------+
        //                   |                             |
        //                   v                             v
        //
        // Index:    0       1       2                     0
        //       +-------+-------+-------+             +-------+
        //       |  100  |  200  |  300  |             |  400  |
        //       +-------+-------+-------+-------+     +-------+-------+
        //       |   a   |   b   |   c   |    ---|---->|   d   |    ---|---->
        //       +-------+-------+-------+-------+     +-------+-------+
        //
        //
        // - After the rebalance:
        //   Left sibling's rightmost key (300) is moved into the right sibling.
        //   Right sibling's new leftmost key (300) is copied into its parent.
        //   Left sibling's rightmost value is moved into the right sibling.
        //
        // Index:                           0
        //                              +-------+
        //                              |  300  |
        //                          +---+---+---+---+
        //                          |       |       |
        //                          +-------+-------+
        //                              |       |
        //                   +----------+       +----------+
        //                   |                             |
        //                   v                             v
        //
        // Index:        0       1                     0       1
        //           +-------+-------+             +-------+-------+
        //           |  100  |  200  |             |  300  |  400  |
        //           +-------+-------+-------+     +-------+-------+-------+
        //           |   a   |   b   |    ---|---->|   c   |   d   |    ---|---->
        //           +-------+-------+-------+     +-------+-------+-------+
        //

        if (kValueSize > 0) {
          // Shift keys and values one position to the right.
          value_type* zvalues = z->_M_values;

          for (uint16_t j = z->_M_header->count; j > 0; j--) {
            zkeys[j] = util::move(zkeys[j - 1]);
            zvalues[j] = util::move(zvalues[j - 1]);
          }

          // Move rightmost value from left sibling into 'z'.
          zvalues[0] = util::move(y->_M_values[ycount - 1]);
        } else {
          // Shift keys one position to the right.
          for (uint16_t j = z->_M_header->count; j > 0; j--) {
            zkeys[j] = util::move(zkeys[j - 1]);
          }
        }

        // Move rightmost key from left sibling into 'z'.
        zkeys[0] = util::move(ykeys[ycount - 1]);

        xkeys[i].key_type::~key_type();
        new (&xkeys[i]) key_type();

        // Copy new leftmost key from right sibling up into 'x'.
        xkeys[i] = zkeys[0];
      }

      y->_M_header->count--;
      z->_M_header->count++;
    }

    template<typename _Parameters>
    void btree<_Parameters>::node::rebalance_right_to_left(node* x, uint16_t i)
    {
      node* y = x->_M_children[i];
      node* z = x->_M_children[i + 1]; // Right sibling.

      uint16_t ycount = y->_M_header->count;
      uint16_t zcount = z->_M_header->count;

      key_type* xkeys = x->_M_keys;
      key_type* ykeys = y->_M_keys;
      key_type* zkeys = z->_M_keys;

      // If 'y' is an internal node...
      if (y->_M_header->type == kInternal) {
        // - Before the rebalance:
        //
        // Index:                           0
        //                              +-------+
        //                              |  200  |
        //                          +---+---+---+---+
        //                          |       |       |
        //                          +-------+-------+
        //                              |       |
        //                   +----------+       +----------+
        //                   |                             |
        //                   v                             v
        //
        // Index:            0                     0       1       2
        //               +-------+             +-------+-------+-------+
        //               |  100  |             |  300  |  400  |  500  |
        //           +---+---+---+---+     +---+---+---+---+---+---+---+---+
        //           |       |       |     |       |       |       |       |
        //           +-------+-------+     +-------+-------+-------+-------+
        //               |       |             |       |       |       |
        //              <100    <200          <300    <400    <500   >=500
        //
        //
        // - After the rebalance:
        //   Parent's key (200) is moved down into the left sibling.
        //   Right sibling's leftmost key (300) is moved up into its parent.
        //   Right sibling's leftmost child pointer is moved into the
        //   left sibling.
        //
        // Index:                           0
        //                              +-------+
        //                              |  300  |
        //                          +---+---+---+---+
        //                          |       |       |
        //                          +-------+-------+
        //                              |       |
        //                   +----------+       +----------+
        //                   |                             |
        //                   v                             v
        //
        // Index:        0       1                     0       1
        //           +-------+-------+             +-------+-------+
        //           |  100  |  200  |             |  400  |  500  |
        //       +---+---+---+---+---+---+     +---+---+---+---+---+---+
        //       |       |       |       |     |       |       |       |
        //       +-------+-------+-------+     +-------+-------+-------+
        //           |       |       |             |       |       |
        //          <100    <200    <300          <400    <500   >=500
        //

        // Move key from 'x' down into 'y'.
        ykeys[ycount] = util::move(xkeys[i]);

        // Move leftmost key from right sibling up into 'x'.
        xkeys[i] = util::move(zkeys[0]);

        node** zchildren = z->_M_children;

        // Move leftmost child pointer from right sibling into 'y'.
        y->_M_children[ycount + 1] = zchildren[0];

        // Shift keys and pointers one position to the left.
        for (uint16_t j = 1; j < zcount; j++) {
          zkeys[j - 1] = util::move(zkeys[j]);
          zchildren[j - 1] = zchildren[j];
        }

        zchildren[zcount - 1] = zchildren[zcount];
      } else {
        // 'y' is a leaf node.

        // - Before the rebalance:
        //
        // Index:                           0
        //                              +-------+
        //                              |  200  |
        //                          +---+---+---+---+
        //                          |       |       |
        //                          +-------+-------+
        //                              |       |
        //                   +----------+       +----------+
        //                   |                             |
        //                   v                             v
        //
        // Index:            0                     0       1       2
        //               +-------+             +-------+-------+-------+
        //               |  100  |             |  200  |  300  |  400  |
        //               +-------+-------+     +-------+-------+-------+-------+
        //               |   a   |    ---|---->|   b   |   c   |   d   |    ---|---->
        //               +-------+-------+     +-------+-------+-------+-------+
        //
        //
        // - After the rebalance:
        //   Right sibling's leftmost key (200) is moved into the left sibling.
        //   Right sibling's new leftmost key (300) is copied into its parent.
        //   Right sibling's leftmost value is moved into the left sibling.
        //
        // Index:                           0
        //                              +-------+
        //                              |  300  |
        //                          +---+---+---+---+
        //                          |       |       |
        //                          +-------+-------+
        //                              |       |
        //                   +----------+       +----------+
        //                   |                             |
        //                   v                             v
        //
        // Index:        0       1                     0       1
        //           +-------+-------+             +-------+-------+
        //           |  100  |  200  |             |  300  |  400  |
        //           +-------+-------+-------+     +-------+-------+-------+
        //           |   a   |   b   |    ---|---->|   c   |   d   |    ---|---->
        //           +-------+-------+-------+     +-------+-------+-------+
        //

        // Move lefmost key from right sibling into 'y'.
        ykeys[ycount] = util::move(zkeys[0]);

        if (kValueSize > 0) {
          value_type* zvalues = z->_M_values;

          // Move leftmost value from right sibling into 'y'.
          y->_M_values[ycount] = util::move(zvalues[0]);

          // Shift keys and values one position to the left.
          for (uint16_t j = 1; j < zcount; j++) {
            zkeys[j - 1] = util::move(zkeys[j]);
            zvalues[j - 1] = util::move(zvalues[j]);
          }
        } else {
          // Shift keys one position to the left.
          for (uint16_t j = 1; j < zcount; j++) {
            zkeys[j - 1] = util::move(zkeys[j]);
          }
        }

        xkeys[i].key_type::~key_type();
        new (&xkeys[i]) key_type();

        // Copy new leftmost key from right sibling up into 'x'.
        xkeys[i] = zkeys[0];
      }

      y->_M_header->count++;
      z->_M_header->count--;
    }

    template<typename _Parameters>
    void btree<_Parameters>::node::merge(node* x, node* y, node* z, uint16_t i)
    {
      uint16_t ycount = y->_M_header->count;
      uint16_t zcount = z->_M_header->count;

      key_type* xkeys = x->_M_keys;
      key_type* ykeys = y->_M_keys;
      key_type* zkeys = z->_M_keys;

      // If 'y' is an internal node...
      if (y->_M_header->type == kInternal) {
        // - Before the merge:
        //
        // Index:                           0       1
        //                              +-------+-------+
        //                              |  200  |  400  |
        //                          +---+---+---+---+---+---+
        //                          |       |       |       |
        //                          +-------+-------+-------+
        //                              |       |       |
        //               +--------------+       |       +--------------+
        //               |                      |                      |
        //               v                      v                      v
        //
        // Index:        0                      0                  0       1
        //           +-------+              +-------+          +-------+-------+
        //           |  100  |              |  300  |          |  500  |  600  |
        //       +---+---+---+---+      +---+---+---+---+  +---+---+---+---+---+---+
        //       |       |       |      |       |       |  |       |       |       |
        //       +-------+-------+      +-------+-------+  +-------+-------+-------+
        //           |       |              |       |          |       |       |
        //          <100    <200           <300    <400       <500    <600   >=600
        //
        //
        // - After the merge:
        //   Parent's key (200) is moved down into the left sibling.
        //   Right sibling's keys (300) are moved into the left sibling.
        //   Right sibling's children pointers are moved into the
        //   left sibling.
        //
        // Index:                             0
        //                                +-------+
        //                                |  400  |
        //                            +---+---+---+---+
        //                            |       |       |
        //                            +-------+-------+
        //                                |       |
        //                     +----------+       +-----------+
        //                     |                              |
        //                     v                              v
        //
        // Index:      0       1       2                  0       1
        //         +-------+-------+-------+          +-------+-------+
        //         |  100  |  200  |  300  |          |  500  |  600  |
        //     +---+---+---+---+---+---+---+---+  +---+---+---+---+---+---+
        //     |       |       |       |       |  |       |       |       |
        //     +-------+-------+-------+-------+  +-------+-------+-------+
        //         |       |       |       |          |       |       |
        //        <100    <200    <300    <400       <500    <600   >=600
        //

        // Move key from 'x' down into 'y'.
        ykeys[ycount++] = util::move(xkeys[i]);

        // Move keys and pointers from right sibling into 'y'.
        node** ychildren = y->_M_children;
        node** zchildren = z->_M_children;
        for (uint16_t j = 0; j < zcount; j++) {
          ykeys[ycount] = util::move(zkeys[j]);
          ychildren[ycount] = zchildren[j];

          ycount++;
        }

        ychildren[ycount] = zchildren[zcount];
      } else {
        // 'y' is a leaf node.

        // - Before the merge:
        //
        // Index:                     0       1
        //                        +-------+-------+
        //                        |  200  |  300  |
        //                    +---+---+---+---+---+---+
        //                    |       |       |       |
        //                    +-------+-------+-------+
        //                        |       |       |
        //          +-------------+       |       +-----------------+
        //          |                     |                         |
        //          v                     v                         v
        //
        // Index:   0                     0                     0       1
        //      +-------+             +-------+             +-------+-------+
        //      |  100  |             |  200  |             |  300  |  400  |
        //      +-------+-------+     +-------+-------+     +-------+-------+-------+
        //      |   a   |    ---|---->|   b   |    ---|---->|   c   |   d   |    ---|---->
        //      +-------+-------+     +-------+-------+     +-------+-------+-------+
        //
        //
        // - After the merge:
        //   Right sibling's keys (200) are moved into the left sibling.
        //   Right sibling's values are moved into the left sibling.
        //
        // Index:                           0
        //                              +-------+
        //                              |  300  |
        //                          +---+---+---+---+
        //                          |       |       |
        //                          +-------+-------+
        //                              |       |
        //                   +----------+       +----------+
        //                   |                             |
        //                   v                             v
        //
        // Index:        0       1                     0       1
        //           +-------+-------+             +-------+-------+
        //           |  100  |  200  |             |  300  |  400  |
        //           +-------+-------+-------+     +-------+-------+-------+
        //           |   a   |   b   |    ---|---->|   c   |   d   |    ---|---->
        //           +-------+-------+-------+     +-------+-------+-------+
        //

        if (kValueSize > 0) {
          value_type* yvalues = y->_M_values;
          value_type* zvalues = z->_M_values;

          // Move right sibling's keys and values to 'y'.
          for (uint16_t j = 0; j < zcount; j++) {
            ykeys[ycount] = util::move(zkeys[j]);
            yvalues[ycount] = util::move(zvalues[j]);

            ycount++;
          }
        } else {
          // Move right sibling's keys to 'y'.
          for (uint16_t j = 0; j < zcount; j++) {
            ykeys[ycount] = util::move(zkeys[j]);
            ycount++;
          }
        }

        xkeys[i].key_type::~key_type();
        new (&xkeys[i]) key_type();

        if (z->next()) {
          z->next()->prev(y);
        }

        y->next(z->next());
      }

      // Shift keys and pointers in 'x' one position to the left.
      uint16_t xcount = x->_M_header->count;
      node** xchildren = x->_M_children;
      for (++i; i < xcount; i++) {
        xkeys[i - 1] = util::move(xkeys[i]);
        xchildren[i] = xchildren[i + 1];
      }

      x->_M_header->count--;
      y->_M_header->count = ycount;

      // Delete 'z'.
      z->_M_header->type = kLeaf;
      z->_M_header->count = 0;

      delete z;
    }

    template<typename _Parameters>
    typename btree<_Parameters>::node::operation_result
    btree<_Parameters>::node::try_rebalance_or_merge_subtree(node* x,
                                                             node*& root,
                                                             uint16_t i)
    {
      operation_result opres;
      switch ((opres = try_rebalance_or_merge(x, root, i))) {
        case kMerged:
        case kShrinked:
          return opres;
        default:
          ;
      }

      x = x->_M_children[i];
      i = 0;

      while (x->_M_header->type == kInternal) {
        try_rebalance_or_merge(x, root, i);
        x = x->_M_children[0];
      }

      return opres;
    }

    template<typename _Parameters>
    typename btree<_Parameters>::node::operation_result
    btree<_Parameters>::node::try_rebalance_or_merge(node* x,
                                                     node*& root,
                                                     uint16_t& i)
    {
      // If the child has the minimum number of keys...
      if (x->_M_children[i]->minkeys()) {
        // If not the leftmost child...
        if (i > 0) {
          // If we can borrow a key from the left sibling...
          if (!x->_M_children[i - 1]->minkeys()) {
            rebalance_left_to_right(x, i);

            return kRebalancedLeftToRight;
          } else if ((i < x->_M_header->count) &&
                     (!x->_M_children[i + 1]->minkeys())) {
            // We can borrow a key from the right sibling.
            rebalance_right_to_left(x, i);

            return kRebalancedRightToLeft;
          } else {
            i--;
            merge(x, x->_M_children[i], x->_M_children[i + 1], i);

            // If the node is empty...
            if (x->_M_header->count == 0) {
              // Set new root.
              root = x->_M_children[0];

              x->_M_header->type = kLeaf;
              delete x;

              return kShrinked;
            }

            return kMerged;
          }
        } else {
          // If we can borrow a key from the right sibling...
          if (!x->_M_children[i + 1]->minkeys()) {
            rebalance_right_to_left(x, i);

            return kRebalancedRightToLeft;
          } else {
            merge(x, x->_M_children[i], x->_M_children[i + 1], i);

            // If the node is empty...
            if (x->_M_header->count == 0) {
              // Set new root.
              root = x->_M_children[0];

              x->_M_header->type = kLeaf;
              delete x;

              return kShrinked;
            }

            return kMerged;
          }
        }
      }

      return kNoop;
    }

    template<typename _Parameters>
    inline const typename btree<_Parameters>::iterator::key_type&
    btree<_Parameters>::iterator::key() const
    {
      return _M_node->_M_keys[_M_pos];
    }

    template<typename _Parameters>
    inline typename btree<_Parameters>::iterator::value_type&
    btree<_Parameters>::iterator::value()
    {
      return _M_node->_M_values[_M_pos];
    }

    template<typename _Parameters>
    inline
    bool btree<_Parameters>::iterator::operator==(const iterator& other) const
    {
      return ((_M_node == other._M_node) && (_M_pos == other._M_pos));
    }

    template<typename _Parameters>
    inline
    bool btree<_Parameters>::iterator::operator!=(const iterator& other) const
    {
      return ((_M_node != other._M_node) || (_M_pos != other._M_pos));
    }

    template<typename _Parameters>
    inline const typename btree<_Parameters>::const_iterator::key_type&
    btree<_Parameters>::const_iterator::key() const
    {
      return _M_node->_M_keys[_M_pos];
    }

    template<typename _Parameters>
    inline const typename btree<_Parameters>::const_iterator::value_type&
    btree<_Parameters>::const_iterator::value() const
    {
      return _M_node->_M_values[_M_pos];
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::const_iterator::
    operator==(const const_iterator& other) const
    {
      return ((_M_node == other._M_node) && (_M_pos == other._M_pos));
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::const_iterator::
    operator!=(const const_iterator& other) const
    {
      return ((_M_node != other._M_node) || (_M_pos != other._M_pos));
    }

    template<typename _Parameters>
    inline btree<_Parameters>::btree(const key_compare& comp)
      : _M_comp(comp),
        _M_root(NULL),
        _M_nkeys(0)
    {
    }

    template<typename _Parameters>
    inline btree<_Parameters>::~btree()
    {
      clear();
    }

    template<typename _Parameters>
    inline void btree<_Parameters>::clear()
    {
      if (_M_root) {
        delete _M_root;
        _M_root = NULL;
      }

      _M_nkeys = 0;
    }

    template<typename _Parameters>
    inline size_t btree<_Parameters>::count() const
    {
      return _M_nkeys;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::insert(const key_type& key,
                                           const value_type& value)
    {
      // If the tree is empty...
      if (!_M_root) {
        if ((_M_root = node::create(node::kLeaf)) == NULL) {
          return false;
        }
      } else if (_M_root->full()) {
        // The root node is full.
        node* s;
        if ((s = node::create(node::kInternal)) == NULL) {
          return false;
        }

        s->_M_children[0] = _M_root;

        if (!s->split_child(0)) {
          delete s;
          return false;
        }

        _M_root = s;
      }

      if (!node::insert_non_full(_M_root, key, value, _M_comp, _M_nkeys)) {
        return false;
      }

      return true;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::erase(const key_type& key)
    {
      // If the tree is empty...
      if (!_M_root) {
        return false;
      }

      if (!node::erase(_M_root, key, _M_comp)) {
        return false;
      }

      if (--_M_nkeys == 0) {
        delete _M_root;
        _M_root = NULL;
      }

      return true;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::get(const key_type& key,
                                        value_type& value) const
    {
      const_iterator it;
      if (!lower_bound(key, it)) {
        return false;
      }

      value = it._M_node->_M_values[it._M_pos];

      return true;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::begin(iterator& it)
    {
      // If the tree is empty...
      if (_M_nkeys == 0) {
        return false;
      }

      it._M_node = _M_root;

      while (it._M_node->_M_header->type == node::kInternal) {
        it._M_node = it._M_node->_M_children[0];
      }

      it._M_pos = 0;

      return true;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::begin(const_iterator& it) const
    {
      // If the tree is empty...
      if (_M_nkeys == 0) {
        return false;
      }

      it._M_node = _M_root;

      while (it._M_node->_M_header->type == node::kInternal) {
        it._M_node = it._M_node->_M_children[0];
      }

      it._M_pos = 0;

      return true;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::end(iterator& it)
    {
      // If the tree is empty...
      if (_M_nkeys == 0) {
        return false;
      }

      it._M_node = _M_root;

      while (it._M_node->_M_header->type == node::kInternal) {
        it._M_node = it._M_node->_M_children[it._M_node->_M_header->count];
      }

      it._M_pos = it._M_node->_M_header->count - 1;

      return true;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::end(const_iterator& it) const
    {
      // If the tree is empty...
      if (_M_nkeys == 0) {
        return false;
      }

      it._M_node = _M_root;

      while (it._M_node->_M_header->type == node::kInternal) {
        it._M_node = it._M_node->_M_children[it._M_node->_M_header->count];
      }

      it._M_pos = it._M_node->_M_header->count - 1;

      return true;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::prev(iterator& it)
    {
      if (it._M_pos > 0) {
        it._M_pos--;
      } else if (it._M_node->prev()) {
        it._M_node = it._M_node->prev();
        it._M_pos = it._M_node->_M_header->count - 1;
      } else {
        return false;
      }

      return true;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::prev(const_iterator& it) const
    {
      if (it._M_pos > 0) {
        it._M_pos--;
      } else if (it._M_node->prev()) {
        it._M_node = it._M_node->prev();
        it._M_pos = it._M_node->_M_header->count - 1;
      } else {
        return false;
      }

      return true;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::next(iterator& it)
    {
      if (it._M_pos < it._M_node->_M_header->count - 1) {
        it._M_pos++;
      } else if (it._M_node->next()) {
        it._M_node = it._M_node->next();
        it._M_pos = 0;
      } else {
        return false;
      }

      return true;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::next(const_iterator& it) const
    {
      if (it._M_pos < it._M_node->_M_header->count - 1) {
        it._M_pos++;
      } else if (it._M_node->next()) {
        it._M_node = it._M_node->next();
        it._M_pos = 0;
      } else {
        return false;
      }

      return true;
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::find(const key_type& key, iterator& it)
    {
      return lower_bound(key, it);
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::find(const key_type& key,
                                         const_iterator& it) const
    {
      return lower_bound(key, it);
    }

    template<typename _Parameters>
    bool btree<_Parameters>::lower_bound(const key_type& key, iterator& it)
    {
      // If the tree is empty...
      if (_M_nkeys == 0) {
        return false;
      }

      bool search_in_next_node = false;

      it._M_node = _M_root;

      while (it._M_node->_M_header->type == node::kInternal) {
        if (it._M_node->lower_bound(key, _M_comp, it._M_pos)) {
          if (!kDuplicates) {
            it._M_pos++;
          } else {
            search_in_next_node = true;
          }
        }

        it._M_node = it._M_node->_M_children[it._M_pos];
      }

      if (it._M_node->lower_bound(key, _M_comp, it._M_pos)) {
        return true;
      }

      if ((!kDuplicates) || (!search_in_next_node)) {
        return false;
      }

      it._M_node = it._M_node->next();

      if (_M_comp(key, it._M_node->_M_keys[0]) != 0) {
        return false;
      }

      it._M_pos = 0;

      return true;
    }

    template<typename _Parameters>
    bool btree<_Parameters>::lower_bound(const key_type& key,
                                         const_iterator& it) const
    {
      // If the tree is empty...
      if (_M_nkeys == 0) {
        return false;
      }

      bool search_in_next_node = false;

      it._M_node = _M_root;

      while (it._M_node->_M_header->type == node::kInternal) {
        if (it._M_node->lower_bound(key, _M_comp, it._M_pos)) {
          if (!kDuplicates) {
            it._M_pos++;
          } else {
            search_in_next_node = true;
          }
        }

        it._M_node = it._M_node->_M_children[it._M_pos];
      }

      if (it._M_node->lower_bound(key, _M_comp, it._M_pos)) {
        return true;
      }

      if ((!kDuplicates) || (!search_in_next_node)) {
        return false;
      }

      it._M_node = it._M_node->next();

      if (_M_comp(key, it._M_node->_M_keys[0]) != 0) {
        return false;
      }

      it._M_pos = 0;

      return true;
    }

    template<typename _Parameters>
    bool btree<_Parameters>::upper_bound(const key_type& key, iterator& it)
    {
      // If the tree is empty...
      if (_M_nkeys == 0) {
        return false;
      }

      it._M_node = _M_root;

      while (it._M_node->_M_header->type == node::kInternal) {
        it._M_node->upper_bound(key, _M_comp, it._M_pos);
        it._M_node = it._M_node->_M_children[it._M_pos];
      }

      return it._M_node->upper_bound(key, _M_comp, it._M_pos);
    }

    template<typename _Parameters>
    bool btree<_Parameters>::upper_bound(const key_type& key,
                                         const_iterator& it) const
    {
      // If the tree is empty...
      if (_M_nkeys == 0) {
        return false;
      }

      it._M_node = _M_root;

      while (it._M_node->_M_header->type == node::kInternal) {
        it._M_node->upper_bound(key, _M_comp, it._M_pos);
        it._M_node = it._M_node->_M_children[it._M_pos];
      }

      return it._M_node->upper_bound(key, _M_comp, it._M_pos);
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::equal_range(const key_type& key,
                                                iterator& begin,
                                                iterator& end)
    {
      return ((lower_bound(key, begin)) && (upper_bound(key, end)));
    }

    template<typename _Parameters>
    inline bool btree<_Parameters>::equal_range(const key_type& key,
                                                const_iterator& begin,
                                                const_iterator& end) const
    {
      return ((lower_bound(key, begin)) && (upper_bound(key, end)));
    }
  }
}

#endif // UTIL_BTREE_H
