#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

template <typename K, typename V> struct Node {
  bool isLeaf;

  std::vector<K> keys;

  // leaf nodes use this stores actual values
  std::vector<V> values;

  // only used by "parent"/internal nodes -> how we traverse the tree
  std::vector<Node<K, V> *> children;

  // leaf nodes will have a linked list for the row just to have quick queries
  // when making a command into database
  Node<K, V> *nextLeaf;

  // specifying a specific constructor for a node
  Node(bool leaf) : isLeaf(leaf), nextLeaf(nullptr) {}

  // empty constructor
  Node() {}
};

class BPlusTree {
private:
  Node<std::string, std::string> *root =
      new Node<std::string, std::string>(true);

  int treeOrder;

public:
  // constructor
  BPlusTree(int treeOrder) : treeOrder(treeOrder) {}

  ~BPlusTree() { delete root; }

  Node<std::string, std::string> *traverseTree(const std::string &key) {
    Node<std::string, std::string> *currentNode = root;

    // while we have not found the leaf nodes continue traversing
    while (!currentNode->isLeaf) {
      // go through the keys that we have stored at this node
      int index = 0;

      // traverse till we either find a key that is larger than our key we are
      // looking for or we reach the end of the keys vector
      while (index < currentNode->keys.size() &&
             key >= currentNode->keys[index])
        index++;

      currentNode = currentNode->children[index];
    } // can further improve by using upper_bound()

    return currentNode;
  }

  void insertSortedOrder(Node<std::string, std::string> *node,
                         const std::string &key, const std::string &value) {
    auto nextPosition =
        std::lower_bound(node->keys.begin(), node->keys.end(), key);

    int index = std::distance(node->keys.begin(), nextPosition);

    if (nextPosition == node->keys.end()) {
      node->keys.push_back(key);
      node->values.push_back(value);
      return;
    }

    node->keys.insert(nextPosition, key);
    node->values.insert(node->values.begin() + index, value);
  }

  void recursiveInsert(Node<std::string, std::string> *node,
                       const std::string &key, const std::string &value) {}

  // main 2 functions
  void insert(const std::string &key, const std::string &value) {
    // to handle node splitting and such will use recursion
  }

  std::optional<std::string> search(const std::string &key) {
    Node<std::string, std::string> *leafNode = traverseTree(key);

    // now see if we can find the specific key we were looking for
    auto matchingElement =
        std::find(leafNode->keys.begin(), leafNode->keys.end(), key);

    // element does not exist
    if (matchingElement == leafNode->keys.end())
      return std::nullopt;

    // element does exist
    int index = matchingElement - leafNode->keys.begin();
    return leafNode->values[index];
  }
};
