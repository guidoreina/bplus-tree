#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include "util/btree/btree_set.h"
#include "util/minus.h"
#include "util/random_generator.h"

static const int kNodeSize = 256;
static const int kNumberKeys = 10 * 1000;

// Compare two strings according to their numeric values.
struct strcomp {
  int operator()(const std::string& x, const std::string& y) const
  {
    return (atoi(x.c_str()) - atoi(y.c_str()));
  }
};

typedef util::btree::btree_set<std::string,
                               strcomp,
                               kNodeSize> string_set_type;

typedef util::btree::btree_set<std::string,
                               strcomp,
                               kNodeSize>::const_iterator
                               string_set_iterator_type;

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

bool string_set_tests()
{
  printf("\nPerforming string set tests...\n");
  string_set_type string_set;
  if (!perform_tests<string_set_type, string_set_iterator_type>(string_set)) {
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
    std::stringstream ss;
    ss << i;
    std::string key(ss.str());

    if (!tree.insert(key)) {
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
    std::stringstream ss;
    ss << i;
    std::string key(ss.str());

    if (!tree.insert(key)) {
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

  std::stringstream ss;
  ss << key;
  std::string k(ss.str());

  if (!tree.insert(k)) {
    printf("[middle_insert] Couldn't insert key: (%d).\n", key);
    return false;
  }

  for (int i = 1; i < kNumberKeys / 2; i++) {
    key = (kNumberKeys / 2) - i;

    ss.str(std::string());
    ss << key;
    k = ss.str();

    if (!tree.insert(k)) {
      printf("[middle_insert] Couldn't insert key: (%d).\n", key);
      return false;
    }

    key = (kNumberKeys / 2) + i;

    ss.str(std::string());
    ss << key;
    k = ss.str();

    if (!tree.insert(k)) {
      printf("[middle_insert] Couldn't insert key: (%d).\n", key);
      return false;
    }
  }

  key = kNumberKeys;

  ss.str(std::string());
  ss << key;
  k = ss.str();

  if (!tree.insert(k)) {
    printf("[middle_insert] Couldn't insert key: (%d).\n", key);
    return false;
  }

  return true;
}

template<typename tree_type, typename iterator_type>
bool forward_erase(tree_type& tree)
{
  for (int i = 1; i <= kNumberKeys; i++) {
    std::stringstream ss;
    ss << i;
    std::string key(ss.str());

    if (!tree.erase(key)) {
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
    std::stringstream ss;
    ss << i;
    std::string key(ss.str());

    if (!tree.erase(key)) {
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
  std::stringstream ss;
  ss << key;
  std::string k(ss.str());

  if (!tree.erase(k)) {
    printf("[middle_erase] Couldn't erase key: (%d).\n", key);
    return false;
  }

  for (int i = 1; i < kNumberKeys / 2; i++) {
    key = (kNumberKeys / 2) - i;
    ss.str(std::string());
    ss << key;
    k = ss.str();

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

  key = kNumberKeys;
  ss.str(std::string());
  ss << key;
  k = ss.str();

  if (!tree.erase(k)) {
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

    std::stringstream ss;
    ss << rnd;
    std::string key(ss.str());

    if (!tree.insert(key)) {
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

    std::stringstream ss;
    ss << rnd;
    std::string k(ss.str());

    ss.str(std::string());
    ss << rnd;
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

    std::stringstream ss;
    ss << rnd;
    std::string k(ss.str());

    ss.str(std::string());
    ss << rnd;
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
    std::stringstream ss;
    ss << rnd;
    std::string k(ss.str());

    if (!tree.find(k, it)) {
      printf("Key (%d) not found.\n", static_cast<int>(rnd));
      return false;
    }

    ss.str(std::string());
    ss << rnd;
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
    std::stringstream ss;
    ss << rnd;
    std::string k(ss.str());

    if (!tree.erase(k)) {
      printf("Key (%d) not found.\n", static_cast<int>(rnd));
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
    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    ss.str(std::string());
    ss << key;
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
    std::stringstream ss;
    ss << key;
    std::string k(ss.str());

    ss.str(std::string());
    ss << key;
    std::string v(ss.str());

    if ((it.key() != k) || (it.value() != v)) {
      printf("Invalid (key, value) (%s, %s), expected (%s, %s).\n",
             it.key().c_str(),
             it.value().c_str(),
             k.c_str(),
             v.c_str());

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

  std::stringstream ss;
  ss << key;
  std::string k(ss.str());

  while (tree.find(k, it)) {
    ss.str(std::string());
    ss << key;
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
