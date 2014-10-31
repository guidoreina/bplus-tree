#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <list>
#include "util/btree/btree_map.h"
#include "util/minus.h"
#include "util/random_generator.h"
#include "int_map_tests.h"

static const int kNodeSize = 256;
static const int kNumberKeys = 100 * 1000;
static const int kNumberRepetitions = 3;

typedef util::btree::btree_map<int,
                               int,
                               util::minus<int>,
                               kNodeSize> int_map_type;

typedef util::btree::btree_map<int,
                               int,
                               util::minus<int>,
                               kNodeSize>::const_iterator int_map_iterator_type;

typedef util::btree::btree_multimap<int,
                                    int,
                                    util::minus<int>,
                                    kNodeSize> int_multimap_type;

typedef util::btree::btree_multimap<int,
                                    int,
                                    util::minus<int>,
                                    kNodeSize>::const_iterator
                                    int_multimap_iterator_type;

template<typename tree_type, typename iterator_type>
static bool perform_tests(tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool forward_insert(tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool backward_insert(tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool middle_insert(tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool forward_erase(tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool backward_erase(tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool middle_erase(tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool test_random(tree_type& tree);

template<typename tree_type, typename iterator_type>
static bool equal(const tree_type& tree,
                  const std::list<std::pair<int, int>>& list);

template<typename tree_type, typename iterator_type>
static bool test_mix(tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool iterate(const tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool reverse_iterate(const tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool find(const tree_type& tree, int number_repetitions);

bool int_map_tests()
{
  printf("\nPerforming int map tests...\n");
  int_map_type int_map;
  if (!perform_tests<int_map_type, int_map_iterator_type>(int_map, 1)) {
    return false;
  }

  printf("\nPerforming int multimap tests...\n");
  int_multimap_type int_multimap;
  if (!perform_tests<int_multimap_type,
                     int_multimap_iterator_type>(int_multimap,
                                                 kNumberRepetitions)) {
    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool perform_tests(tree_type& tree, int number_repetitions)
{
  const char* types[] = {"forward", "backward", "middle"};

  for (size_t i = 0; i < 3; i++) {
    printf("Inserting %d (key, value) pairs (%s)...\n",
           kNumberKeys,
           types[i]);

    switch (i) {
      case 0:
        if (!forward_insert<tree_type, iterator_type>(tree,
                                                      number_repetitions)) {
          return false;
        }

        break;
      case 1:
        if (!backward_insert<tree_type, iterator_type>(tree,
                                                       number_repetitions)) {
          return false;
        }

        break;
      case 2:
        if (!middle_insert<tree_type, iterator_type>(tree,
                                                     number_repetitions)) {
          return false;
        }

        break;
    }

    if (tree.count() != static_cast<size_t>(kNumberKeys * number_repetitions)) {
      printf("Unexpected number of keys (%lu), %d keys expected.\n",
             tree.count(),
             kNumberKeys * number_repetitions);

      return false;
    }

    printf("Iterating (forward)...\n");
    if (!iterate<tree_type, iterator_type>(tree, number_repetitions)) {
      return false;
    }

    printf("Iterating (backward)...\n");
    if (!reverse_iterate<tree_type, iterator_type>(tree, number_repetitions)) {
      return false;
    }

    printf("Finding...\n");
    if (!find<tree_type, iterator_type>(tree, number_repetitions)) {
      return false;
    }

    printf("Erasing %d keys (%s)...\n", kNumberKeys, types[i]);

    switch (i) {
      case 0:
        if (!forward_erase<tree_type, iterator_type>(tree,
                                                     number_repetitions)) {
          return false;
        }

        break;
      case 1:
        if (!backward_erase<tree_type, iterator_type>(tree,
                                                      number_repetitions)) {
          return false;
        }

        break;
      case 2:
        if (!middle_erase<tree_type, iterator_type>(tree,
                                                    number_repetitions)) {
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

  if (!test_mix<tree_type, iterator_type>(tree, number_repetitions)) {
    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool forward_insert(tree_type& tree, int number_repetitions)
{
  int count = 1;

  for (int i = 1; i <= kNumberKeys; i++) {
    for (int j = 1; j <= number_repetitions; j++) {
      if (!tree.insert(i, count)) {
        printf("[forward_insert] Couldn't insert key: (%d, %d).\n",
               i,
               count);

        return false;
      }

      count++;
    }
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool backward_insert(tree_type& tree, int number_repetitions)
{
  for (int i = kNumberKeys; i > 0; i--) {
    for (int j = 1; j <= number_repetitions; j++) {
      int value = ((i - 1) * number_repetitions) + j;

      if (!tree.insert(i, value)) {
        printf("[backward_insert] Couldn't insert key: (%d, %d).\n", i, value);
        return false;
      }
    }
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool middle_insert(tree_type& tree, int number_repetitions)
{
  for (int j = 1; j <= number_repetitions; j++) {
    int key = kNumberKeys / 2;
    int value = ((key - 1) * number_repetitions) + j;

    if (!tree.insert(key, value)) {
      printf("[middle_insert] Couldn't insert key: (%d, %d).\n", key, value);
      return false;
    }
  }

  for (int i = 1; i < kNumberKeys / 2; i++) {
    for (int j = 1; j <= number_repetitions; j++) {
      int key = (kNumberKeys / 2) - i;
      int value = ((key - 1) * number_repetitions) + j;

      if (!tree.insert(key, value)) {
        printf("[middle_insert] Couldn't insert key: (%d, %d).\n", key, value);
        return false;
      }

      key = (kNumberKeys / 2) + i;
      value = ((key - 1) * number_repetitions) + j;

      if (!tree.insert(key, value)) {
        printf("[middle_insert] Couldn't insert key: (%d, %d).\n", key, value);
        return false;
      }
    }
  }

  for (int j = 1; j <= number_repetitions; j++) {
    int key = kNumberKeys;
    int value = ((key - 1) * number_repetitions) + j;

    if (!tree.insert(key, value)) {
      printf("[middle_insert] Couldn't insert key: (%d, %d).\n", key, value);
      return false;
    }
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool forward_erase(tree_type& tree, int number_repetitions)
{
  for (int i = 1; i <= kNumberKeys; i++) {
    for (int j = 1; j <= number_repetitions; j++) {
      if (!tree.erase(i)) {
        printf("[forward_erase] Couldn't erase key: (%d).\n", i);
        return false;
      }
    }
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool backward_erase(tree_type& tree, int number_repetitions)
{
  for (int i = kNumberKeys; i > 0; i--) {
    for (int j = 1; j <= number_repetitions; j++) {
      if (!tree.erase(i)) {
        printf("[backward_erase] Couldn't erase key: (%d).\n", i);
        return false;
      }
    }
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool middle_erase(tree_type& tree, int number_repetitions)
{
  for (int j = 1; j <= number_repetitions; j++) {
    int key = kNumberKeys / 2;
    if (!tree.erase(key)) {
      printf("[middle_erase] Couldn't erase key: (%d).\n", key);
      return false;
    }
  }

  for (int i = 1; i < kNumberKeys / 2; i++) {
    for (int j = 1; j <= number_repetitions; j++) {
      int key = (kNumberKeys / 2) - i;
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
  }

  for (int j = 1; j <= number_repetitions; j++) {
    int key = kNumberKeys;
    if (!tree.erase(key)) {
      printf("[middle_erase] Couldn't erase key: (%d).\n", key);
      return false;
    }
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

    if (!tree.insert(static_cast<int>(rnd), static_cast<int>(rnd))) {
      printf("[test_random] Couldn't insert key: (%d, %d).\n",
             static_cast<int>(rnd),
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
    int value = static_cast<int>(rnd);

    if ((it.key() != key) || (it.value() != value)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             value);

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
    int value = static_cast<int>(rnd);

    if ((it.key() != key) || (it.value() != value)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             value);

      return false;
    }

    i--;
  } while (tree.next(it));

  printf("[test_random] Finding...\n");

  i = 0;
  long rnd;
  while (random_generator.unordered(i, rnd)) {
    int key = static_cast<int>(rnd);
    int value = static_cast<int>(rnd);
    if (!tree.find(key, it)) {
      printf("(key, value) (%d, %d) not found.\n", key, value);
      return false;
    }

    if ((it.key() != key) || (it.value() != value)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             value);

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
bool equal(const tree_type& tree, const std::list<std::pair<int, int>>& list)
{
  if (list.size() != tree.count()) {
    printf("Invalid number of keys.\n");
    return false;
  }

  iterator_type it;
  if (!tree.begin(it)) {
    printf("begin() failed.\n");
    return false;
  }

  std::list<std::pair<int, int>>::const_iterator it_list;
  for (it_list = list.begin(); it_list != list.end(); ++it_list) {
    if ((it.key() != it_list->first) || (it.value() != it_list->second)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             it_list->first,
             it_list->second);

      return false;
    }

    if (!tree.next(it)) {
      int key = it_list->first;

      if (++it_list != list.end()) {
        printf("Not all the keys were found (last key found %d).\n", key);
        return false;
      }

      break;
    }
  }

  it_list = list.begin();
  while (it_list != list.end()) {
    if (!tree.find(it_list->first, it)) {
      printf("Couldn't find (key, value) (%d, %d).\n",
             it_list->first,
             it_list->second);

      return false;
    }

    if ((it.key() != it_list->first) || (it.value() != it_list->second)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             it_list->first,
             it_list->second);

      return false;
    }

    do {
      ++it_list;
    } while ((it_list != list.end()) && (it_list->first == it.key()));
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool test_mix(tree_type& tree, int number_repetitions)
{
  printf("Testing mixed operations...\n");

  // Insert keys.
  std::list<std::pair<int, int>> list;

  int count = 1;
  for (int i = 1; i <= kNumberKeys; i++) {
    for (int j = 1; j <= number_repetitions; j++) {
      list.push_back(std::make_pair(i, count));

      if (!tree.insert(i, count)) {
        printf("[test_mix] Couldn't insert key: (%d, %d).\n",
               i,
               count);

        return false;
      }

      count++;
    }
  }

  if (list.size() != tree.count()) {
    printf("Invalid number of keys.\n");
    return false;
  }

  for (int m = 1; m <= 3; m++) {
    // Remove keys.
    for (int j = 1; j <= number_repetitions; j++) {
      static const int kRemoveSteps[] = {23, 19, 17, 13, 11, 7, 0};

      int remove_step;
      for (int k = 0; ((remove_step = kRemoveSteps[k]) != 0); k++) {
        std::list<std::pair<int, int>>::iterator it_list = list.begin();
        while (it_list != list.end()) {
          int i = it_list->first;
          if ((i % remove_step) == 0) {
            int l;
            for (l = 0; l < k; l++) {
              if (((i % kRemoveSteps[l]) == 0)) {
                break;
              }
            }

            if (l == k) {
              it_list = list.erase(it_list);

              if (!tree.erase(i)) {
                printf("[test_mix] Couldn't erase key: (%d).\n", i);
                return false;
              }

              while ((it_list != list.end()) && (it_list->first == i)) {
                ++it_list;
              }
            } else {
              ++it_list;
            }
          } else {
            ++it_list;
          }
        }

        if (!equal<tree_type, iterator_type>(tree, list)) {
          return false;
        }
      }
    }

    // Insert keys.
    for (int j = 1; j <= number_repetitions; j++) {
      static const int kInsertSteps[] = {23, 19, 17, 13, 11, 7, 0};

      int insert_step;
      for (int k = 0; ((insert_step = kInsertSteps[k]) != 0); k++) {
        std::list<std::pair<int, int>>::iterator it_list = list.begin();
        int key = insert_step;
        while (it_list != list.end()) {
          if (it_list->first > key) {
            int l;
            for (l = 0; l < k; l++) {
              if (((key % kInsertSteps[l]) == 0)) {
                break;
              }
            }

            if (l == k) {
              int value = it_list->second - 1;

              list.insert(it_list, std::make_pair(key, value));

              if (!tree.insert(key, value)) {
                printf("[test_mix] Couldn't insert key: (%d).\n", key);
                return false;
              }

              key += kInsertSteps[k];
            }
          }

          ++it_list;
        }

        if (!equal<tree_type, iterator_type>(tree, list)) {
          return false;
        }
      }
    }
  }

  tree.clear();

  return true;
}

template<typename tree_type, typename iterator_type>
bool iterate(const tree_type& tree, int number_repetitions)
{
  iterator_type it;
  if (!tree.begin(it)) {
    printf("begin() failed.\n");
    return false;
  }

  int key = 1;
  int value = 1;

  do {
    if ((it.key() != key) || (it.value() != value)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             value);

      return false;
    }

    if ((value++ % number_repetitions) == 0) {
      key++;
    }
  } while (tree.next(it));

  if (value - 1 != kNumberKeys * number_repetitions) {
    printf("Invalid number of keys.\n");
    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool reverse_iterate(const tree_type& tree, int number_repetitions)
{
  iterator_type it;
  if (!tree.end(it)) {
    printf("end() failed.\n");
    return false;
  }

  int key = kNumberKeys;
  int value = kNumberKeys * number_repetitions;

  do {
    if ((it.key() != key) || (it.value() != value)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             value);

      return false;
    }

    if ((--value % number_repetitions) == 0) {
      key--;
    }
  } while (tree.prev(it));

  if (value != 0) {
    printf("Invalid number of keys.\n");
    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool find(const tree_type& tree, int number_repetitions)
{
  iterator_type it;

  int key = 1;
  int value = 1;

  while (tree.find(key, it)) {
    if ((it.key() != key) || (it.value() != value)) {
      printf("Invalid (key, value) (%d, %d), expected (%d, %d).\n",
             it.key(),
             it.value(),
             key,
             value);

      return false;
    }

    key++;
    value += number_repetitions;
  }

  if (key - 1 != kNumberKeys) {
    printf("Not all the keys were found (last key found %d).\n", key - 1);
    return false;
  }

  return true;
}
