#pragma once

#include <optional>
#include <string>
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
      Node<std::string, std::string> newCurrent = nullptr;

      for (int i = 0; i < currentNode->keys.size(); i++) {
        // found what direction to traverse
        if (key < currentNode->keys[i]) {
          foundMatch = true;
        }
      }
    }
  }
  // main 2 functions
  void insert(const std::string &key, const std::string &value) {}
  std::optional<std::string> search(const std::string &key) {}
};
