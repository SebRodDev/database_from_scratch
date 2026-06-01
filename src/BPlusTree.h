#pragma once

#include <algorithm>
#include <iterator>
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

  std::pair<std::string, Node<std::string, std::string> *>
  recursiveInsert(Node<std::string, std::string> *node, const std::string &key,
                  const std::string &value) {

    if (node->isLeaf) {
      insertSortedOrder(node, key, value);
      // check if we have to split or not
      if (node->keys.size() <= treeOrder)
        return {"", nullptr};

      // have to split a leaf node because the leaf node became too full
      Node<std::string, std::string> *newNode =
          new Node<std::string, std::string>(true);

      // moving half the keys from current node to newly split
      auto keyMidpoint = node->keys.begin() + (node->keys.size() / 2);
      std::move(keyMidpoint, node->keys.end(),
                std::back_inserter(newNode->keys));
      node->keys.erase(keyMidpoint, node->keys.end());

      // moving half the values to newly split node
      auto valuesMidpoint = node->values.begin() + (node->values.size() / 2);
      std::move(valuesMidpoint, node->values.end(),
                std::back_inserter(newNode->values));
      node->values.erase(valuesMidpoint, node->values.end());

      // updating the pointers
      newNode->nextLeaf = node->nextLeaf;
      node->nextLeaf = newNode;

      return {newNode->keys[0], newNode};
    }

    // now see if we can find the specific key we were looking for
    auto matchingElement = std::find(node->keys.begin(), node->keys.end(), key);

    // for cases where we did not insert right away on first traversal must
    // traverse tree to find which one to go to
    int index = matchingElement - node->keys.begin();
    auto childRes = recursiveInsert(node->children[index], key, value);

    // in the case that we did not have to split the child node then we're done
    // and dont have to continue splitting
    if (childRes.second == nullptr) {
      return {"", nullptr};
    }

    node->keys.insert(node->keys.begin() + index, childRes.first);
    node->children.insert(node->children.begin() + index + 1, childRes.second);

    // now have to check if this internal node has reached capacity if it has
    // then must split if not can stop
    if (node->keys.size() <= treeOrder) {
      return {"", nullptr};
    }

    // internal node split
    // non leaf node splitting which means that we mark it as false for being a
    // leaf node
    Node<std::string, std::string> *newNode =
        new Node<std::string, std::string>(false);

    // choosing how we want to split the internal node and since it can only
    // hold at most treeOrder nodes then this one has to move up
    int splitPoint = treeOrder / 2;

    // must move up a key to represent this value in the node that is not
    // promoted up
    std::string promotedKey = node->keys[splitPoint];

    // move everything to the right of the split point to this new node because
    // that represents our new split
    newNode->keys.assign(node->keys.begin() + splitPoint + 1, node->keys.end());

    newNode->children.assign(node->children.begin() + splitPoint + 1,
                             node->children.end());

    // dont have to copy over the values because this is an internal node values
    // are not stored in the internal nodes only leaf nodes
    node->keys.erase(node->keys.begin() + splitPoint, node->keys.end());
    node->children.erase(node->children.begin() + splitPoint + 1,
                         node->children.end());

    return {promotedKey, newNode};
  }

  // main 2 functions
  void insert(const std::string &key, const std::string &value) {
    // to handle node splitting and such will use recursion
    std::pair<std::string, Node<std::string, std::string> *> result =
        recursiveInsert(root, key, value);

    // handle return
    if (result.second != nullptr) {
      // if we had to split the root because an internal node was split then we
      // must properly update a new root

      Node<std::string, std::string> *newRoot =
          new Node<std::string, std::string>(false);

      // the original root is now a child to this new root
      newRoot->children.push_back(root);

      // add whatever node was just split from the rest and moved upwards
      newRoot->keys.push_back(result.first);
      newRoot->children.push_back(result.second);

      root = newRoot;
    }
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
