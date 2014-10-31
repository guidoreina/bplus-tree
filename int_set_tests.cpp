#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "util/btree/btree_set.h"
#include "util/minus.h"
#include "util/random_generator.h"
#include "int_set_tests.h"

static const int kNodeSize = 256;
static const int kNumberKeys = 100 * 1000;

typedef util::btree::btree_set<int,
                               util::minus<int>,
                               kNodeSize> int_set_type;

typedef util::btree::btree_set<int,
                               util::minus<int>,
                               kNodeSize>::const_iterator int_set_iterator_type;

template<typename tree_type, typename iterator_type>
static bool perform_tests(tree_type& tree);

template<typename tree_type, typename iterator_type>
static bool forward_insert(tree_type& tree);

template<typename tree_type, typename iterator_type>
static bool backward_insert(tree_type& tree);

template<typename tree_type, typename iterator_type>
static bool middle_insert(tree_type& tree);

template<typename tree_type, typename iterator_type>
static bool forward_erase(tree_type& tree);

template<typename tree_type, typename iterator_type>
static bool backward_erase(tree_type& tree);

template<typename tree_type, typename iterator_type>
static bool middle_erase(tree_type& tree);

template<typename tree_type, typename iterator_type>
static bool test_random(tree_type& tree);

template<typename tree_type, typename iterator_type>
static bool iterate(const tree_type& tree);

template<typename tree_type, typename iterator_type>
static bool reverse_iterate(const tree_type& tree);

template<typename tree_type, typename iterator_type>
static bool find(const tree_type& tree);

bool int_set_tests()
{
  printf("\nPerforming int set tests...\n");
  int_set_type int_set;
  if (!perform_tests<int_set_type, int_set_iterator_type>(int_set)) {
    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool perform_tests(tree_type& tree)
{
  const char* types[] = {"forward", "backward", "middle"};

  for (size_t i = 0; i < 3; i++) {
    printf("Inserting %d (key, value) pairs (%s)...\n",
           kNumberKeys,
           types[i]);

    switch (i) {
      case 0:
        if (!forward_insert<tree_type, iterator_type>(tree)) {
          return false;
        }

        break;
      case 1:
        if (!backward_insert<tree_type, iterator_type>(tree)) {
          return false;
        }

        break;
      case 2:
        if (!middle_insert<tree_type, iterator_type>(tree)) {
          return false;
        }

        break;
    }

    if (tree.count() != static_cast<size_t>(kNumberKeys)) {
      printf("Unexpected number of keys (%lu), %d keys expected.\n",
             tree.count(),
             kNumberKeys);

      return false;
    }

    printf("Iterating (forward)...\n");
    if (!iterate<tree_type, iterator_type>(tree)) {
      return false;
    }

    printf("Iterating (backward)...\n");
    if (!reverse_iterate<tree_type, iterator_type>(tree)) {
      return false;
    }

    printf("Finding...\n");
    if (!find<tree_type, iterator_type>(tree)) {
      return false;
    }

    printf("Erasing %d keys (%s)...\n", kNumberKeys, types[i]);

    switch (i) {
      case 0:
        if (!forward_erase<tree_type, iterator_type>(tree)) {
          return false;
        }

        break;
      case 1:
        if (!backward_erase<tree_type, iterator_type>(tree)) {
          return false;
        }

        break;
      case 2:
        if (!middle_erase<tree_type, iterator_type>(tree)) {
          return false;
        }

        break;
    }

    if (tree.count() != 0) {
      printf("Unexpected number of keys (%lu), %d keys expected.\n",
             tree.count(),
             0);

      return false;
    }
  }

  if (!test_random<tree_type, iterator_type>(tree)) {
    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool forward_insert(tree_type& tree)
{
  for (int i = 1; i <= kNumberKeys; i++) {
    if (!tree.insert(i)) {
      printf("[forward_insert] Couldn't insert key: (%d).\n", i);
      return false;
    }
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool backward_insert(tree_type& tree)
{
  for (int i = kNumberKeys; i > 0; i--) {
    if (!tree.insert(i)) {
      printf("[backward_insert] Couldn't insert key: (%d).\n", i);
      return false;
    }
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool middle_insert(tree_type& tree)
{
  int key = kNumberKeys / 2;
  if (!tree.insert(key)) {
    printf("[middle_insert] Couldn't insert key: (%d).\n", key);
    return false;
  }

  for (int i = 1; i < kNumberKeys / 2; i++) {
    key = (kNumberKeys / 2) - i;
    if (!tree.insert(key)) {
      printf("[middle_insert] Couldn't insert key: (%d).\n", key);
      return false;
    }

    key = (kNumberKeys / 2) + i;
    if (!tree.insert(key)) {
      printf("[middle_insert] Couldn't insert key: (%d).\n", key);
      return false;
    }
  }

  key = kNumberKeys;
  if (!tree.insert(key)) {
    printf("[middle_insert] Couldn't insert key: (%d).\n", key);
    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool forward_erase(tree_type& tree)
{
  for (int i = 1; i <= kNumberKeys; i++) {
    if (!tree.erase(i)) {
      printf("[forward_erase] Couldn't erase key: (%d).\n", i);
      return false;
    }
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool backward_erase(tree_type& tree)
{
  for (int i = kNumberKeys; i > 0; i--) {
    if (!tree.erase(i)) {
      printf("[backward_erase] Couldn't erase key: (%d).\n", i);
      return false;
    }
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool middle_erase(tree_type& tree)
{
  int key = kNumberKeys / 2;
  if (!tree.erase(key)) {
    printf("[middle_erase] Couldn't erase key: (%d).\n", key);
    return false;
  }

  for (int i = 1; i < kNumberKeys / 2; i++) {
    key = (kNumberKeys / 2) - i;
    if (!tree.erase(key)) {
      printf("[middle_erase] Couldn't erase key: (%d).\n", key);
      return false;
    }

    key = (kNumberKeys / 2) + i;
    if (!tree.erase(key)) {
      printf("[middle_erase] Couldn't erase key: (%d).\n", key);
      return false;
    }
  }

  key = kNumberKeys;
  if (!tree.erase(key)) {
    printf("[middle_erase] Couldn't erase key: (%d).\n", key);
    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool test_random(tree_type& tree)
{
  printf("[test_random] Generating %d random numbers...\n", kNumberKeys);

  util::random_generator random_generator;
  if (!random_generator.init(kNumberKeys)) {
    printf("[test_random] Couldn't initialize random generator.\n");
    return false;
  }

  printf("[test_random] Inserting %d random numbers...\n", kNumberKeys);
  for (int i = 0; i < kNumberKeys; i++) {
    long rnd;
    if (!random_generator.unordered(i, rnd)) {
      printf("[test_random] Couldn't generate random number.\n");
      return false;
    }

    if (!tree.insert(static_cast<int>(rnd))) {
      printf("[test_random] Couldn't insert key: (%d).\n",
             static_cast<int>(rnd));

      return false;
    }
  }

  printf("[test_random] Iterating forward...\n");

  iterator_type it;
  if (!tree.begin(it)) {
    printf("begin() failed.\n");
    return false;
  }

  unsigned i = 0;
  do {
    long rnd;
    if (!random_generator.ordered(i, rnd)) {
      printf("[test_random] Couldn't get random number.\n");
      return false;
    }

    int key = static_cast<int>(rnd);

    if ((it.key() != key) || (it.value() != key)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             key);

      return false;
    }

    i++;
  } while (tree.next(it));

  printf("[test_random] Iterating backward...\n");

  if (!tree.end(it)) {
    printf("end() failed.\n");
    return false;
  }

  i = kNumberKeys - 1;
  do {
    long rnd;
    if (!random_generator.ordered(i, rnd)) {
      printf("[test_random] Couldn't get random number.\n");
      return false;
    }

    int key = static_cast<int>(rnd);

    if ((it.key() != key) || (it.value() != key)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             key);

      return false;
    }

    i--;
  } while (tree.next(it));

  printf("[test_random] Finding...\n");

  i = 0;
  long rnd;
  while (random_generator.unordered(i, rnd)) {
    int key = static_cast<int>(rnd);
    if (!tree.find(key, it)) {
      printf("Key (%d) not found.\n", key);
      return false;
    }

    if ((it.key() != key) || (it.value() != key)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             key);

      return false;
    }

    i++;
  }

  if (static_cast<int>(i) != kNumberKeys) {
    printf("Not all the keys were found (last key found %u).\n", i - 1);
    return false;
  }

  printf("[test_random] Erasing...\n");

  i = 0;
  while (random_generator.unordered(i, rnd)) {
    int key = static_cast<int>(rnd);
    if (!tree.erase(key)) {
      printf("Key (%d) not found.\n", key);
      return false;
    }

    i++;
  }

  if (tree.count() != 0) {
    printf("Unexpected number of keys (%lu), %d keys expected.\n",
           tree.count(),
           0);

    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool iterate(const tree_type& tree)
{
  iterator_type it;
  if (!tree.begin(it)) {
    printf("begin() failed.\n");
    return false;
  }

  int key = 1;

  do {
    if ((it.key() != key) || (it.value() != key)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             key);

      return false;
    }

    key++;
  } while (tree.next(it));

  if (key - 1 != kNumberKeys) {
    printf("Invalid number of keys.\n");
    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool reverse_iterate(const tree_type& tree)
{
  iterator_type it;
  if (!tree.end(it)) {
    printf("end() failed.\n");
    return false;
  }

  int key = kNumberKeys;

  do {
    if ((it.key() != key) || (it.value() != key)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             key);

      return false;
    }

    key--;
  } while (tree.prev(it));

  if (key != 0) {
    printf("Invalid number of keys.\n");
    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool find(const tree_type& tree)
{
  iterator_type it;

  int key = 1;

  while (tree.find(key, it)) {
    if ((it.key() != key) || (it.value() != key)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             key);

      return false;
    }

    key++;
  }

  if (key - 1 != kNumberKeys) {
    printf("Not all the keys were found (last key found %d).\n", key - 1);
    return false;
  }

  return true;
}
