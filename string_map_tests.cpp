#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include "util/btree/btree_map.h"
#include "util/minus.h"
#include "util/random_generator.h"

static const int kNodeSize = 256;
static const int kNumberKeys = 10 * 1000;
static const int kNumberRepetitions = 3;

// Compare two strings according to their numeric values.
struct strcomp {
  int operator()(const std::string& x, const std::string& y) const
  {
    return (atoi(x.c_str()) - atoi(y.c_str()));
  }
};

typedef util::btree::btree_map<std::string,
                               std::string,
                               strcomp,
                               kNodeSize> string_map_type;

typedef util::btree::btree_map<std::string,
                               std::string,
                               strcomp,
                               kNodeSize>::const_iterator
                               string_map_iterator_type;

typedef util::btree::btree_multimap<std::string,
                                    std::string,
                                    strcomp,
                                    kNodeSize> string_multimap_type;

typedef util::btree::btree_multimap<std::string,
                                    std::string,
                                    strcomp,
                                    kNodeSize>::const_iterator
                                    string_multimap_iterator_type;

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
static bool iterate(const tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool reverse_iterate(const tree_type& tree, int number_repetitions);

template<typename tree_type, typename iterator_type>
static bool find(const tree_type& tree, int number_repetitions);

bool string_map_tests()
{
  printf("\nPerforming string map tests...\n");
  string_map_type string_map;
  if (!perform_tests<string_map_type, string_map_iterator_type>(string_map,
                                                                1)) {
    return false;
  }

  printf("\nPerforming string multimap tests...\n");
  string_multimap_type string_multimap;
  if (!perform_tests<string_multimap_type,
                     string_multimap_iterator_type>(string_multimap,
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

  return true;
}

template<typename tree_type, typename iterator_type>
bool forward_insert(tree_type& tree, int number_repetitions)
{
  int count = 1;

  for (int i = 1; i <= kNumberKeys; i++) {
    for (int j = 1; j <= number_repetitions; j++) {
      std::stringstream ss;
      ss << i;
      std::string key(ss.str());

      ss.str(std::string());
      ss << count;
      std::string value(ss.str());

      if (!tree.insert(key, value)) {
        printf("[forward_insert] Couldn't insert key: (%d, %d).\n", i, count);
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

      std::stringstream ss;
      ss << i;
      std::string k(ss.str());

      ss.str(std::string());
      ss << value;
      std::string v(ss.str());

      if (!tree.insert(k, v)) {
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

    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    ss.str(std::string());
    ss << value;
    std::string v(ss.str());

    if (!tree.insert(k, v)) {
      printf("[middle_insert] Couldn't insert key: (%d, %d).\n", key, value);
      return false;
    }
  }

  for (int i = 1; i < kNumberKeys / 2; i++) {
    for (int j = 1; j <= number_repetitions; j++) {
      int key = (kNumberKeys / 2) - i;
      int value = ((key - 1) * number_repetitions) + j;

      std::stringstream ss;
      ss << key;
      std::string k(ss.str());

      ss.str(std::string());
      ss << value;
      std::string v(ss.str());

      if (!tree.insert(k, v)) {
        printf("[middle_insert] Couldn't insert key: (%d, %d).\n", key, value);
        return false;
      }

      key = (kNumberKeys / 2) + i;
      value = ((key - 1) * number_repetitions) + j;

      ss.str(std::string());
      ss << key;
      k = ss.str();

      ss.str(std::string());
      ss << value;
      v = ss.str();

      if (!tree.insert(k, v)) {
        printf("[middle_insert] Couldn't insert key: (%d, %d).\n", key, value);
        return false;
      }
    }
  }

  for (int j = 1; j <= number_repetitions; j++) {
    int key = kNumberKeys;
    int value = ((key - 1) * number_repetitions) + j;

    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    ss.str(std::string());
    ss << value;
    std::string v(ss.str());

    if (!tree.insert(k, v)) {
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
      std::stringstream ss;
      ss << i;
      std::string key(ss.str());

      if (!tree.erase(key)) {
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
      std::stringstream ss;
      ss << i;
      std::string key(ss.str());

      if (!tree.erase(key)) {
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
    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    if (!tree.erase(k)) {
      printf("[middle_erase] Couldn't erase key: (%d).\n", key);
      return false;
    }
  }

  for (int i = 1; i < kNumberKeys / 2; i++) {
    for (int j = 1; j <= number_repetitions; j++) {
      int key = (kNumberKeys / 2) - i;
      std::stringstream ss;
      ss << key;
      std::string k(ss.str());

      if (!tree.erase(k)) {
        printf("[middle_erase] Couldn't erase key: (%d).\n", key);
        return false;
      }

      key = (kNumberKeys / 2) + i;
      ss.str(std::string());
      ss << key;
      k = ss.str();

      if (!tree.erase(k)) {
        printf("[middle_erase] Couldn't erase key: (%d).\n", key);
        return false;
      }
    }
  }

  for (int j = 1; j <= number_repetitions; j++) {
    int key = kNumberKeys;
    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    if (!tree.erase(k)) {
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

    std::stringstream ss;
    ss << rnd;
    std::string key(ss.str());

    ss.str(std::string());
    ss << rnd;
    std::string value(ss.str());

    if (!tree.insert(key, value)) {
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

    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    ss.str(std::string());
    ss << value;
    std::string v(ss.str());

    if ((it.key() != k) || (it.value() != v)) {
      printf("Invalid (key, value) (%s, %s), expected (%s, %s).\n",
             it.key().c_str(),
             it.value().c_str(),
             k.c_str(),
             v.c_str());

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

    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    ss.str(std::string());
    ss << value;
    std::string v(ss.str());

    if ((it.key() != k) || (it.value() != v)) {
      printf("Invalid (key, value) (%s, %s), expected (%s, %s).\n",
             it.key().c_str(),
             it.value().c_str(),
             k.c_str(),
             v.c_str());

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

    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    if (!tree.find(k, it)) {
      printf("(key, value) (%d, %d) not found.\n", key, value);
      return false;
    }

    ss.str(std::string());
    ss << value;
    std::string v(ss.str());

    if ((it.key() != k) || (it.value() != v)) {
      printf("Invalid (key, value) (%s, %s), expected (%s, %s).\n",
             it.key().c_str(),
             it.value().c_str(),
             k.c_str(),
             v.c_str());

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

    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    if (!tree.erase(k)) {
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
    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    ss.str(std::string());
    ss << value;
    std::string v(ss.str());

    if ((it.key() != k) || (it.value() != v)) {
      printf("Invalid (key, value) (%s, %s), expected (%s, %s).\n",
             it.key().c_str(),
             it.value().c_str(),
             k.c_str(),
             v.c_str());

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
    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    ss.str(std::string());
    ss << value;
    std::string v(ss.str());

    if ((it.key() != k) || (it.value() != v)) {
      printf("Invalid (key, value) (%s, %s), expected (%s, %s).\n",
             it.key().c_str(),
             it.value().c_str(),
             k.c_str(),
             v.c_str());

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

  std::stringstream ss;
  ss << key;
  std::string k(ss.str());

  while (tree.find(k, it)) {
    ss.str(std::string());
    ss << value;
    std::string v(ss.str());

    if ((it.key() != k) || (it.value() != v)) {
      printf("Invalid (key, value) (%s, %s), expected (%s, %s).\n",
             it.key().c_str(),
             it.value().c_str(),
             k.c_str(),
             v.c_str());

      return false;
    }

    key++;
    value += number_repetitions;

    ss.str(std::string());
    ss << key;
    k = ss.str();
  }

  if (key - 1 != kNumberKeys) {
    printf("Not all the keys were found (last key found %d).\n", key - 1);
    return false;
  }

  return true;
}
