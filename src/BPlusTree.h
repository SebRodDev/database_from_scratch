#pragma once

#include <algorithm>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

template <typename K, typename V>
struct Node {
    bool isLeaf;
    std::vector<K> keys;
    std::vector<V> values;
    std::vector<Node<K, V> *> children;
    Node<K, V> *nextLeaf;

    Node(bool leaf) : isLeaf(leaf), nextLeaf(nullptr) {}
    Node() : isLeaf(true), nextLeaf(nullptr) {}
};

class BPlusTree {
private:
    Node<std::string, std::string> *root = new Node<std::string, std::string>(true);
    int treeOrder;

public:
    BPlusTree(int treeOrder) : treeOrder(treeOrder) {}

    ~BPlusTree() {
        delete root;
    }

    Node<std::string, std::string> *traverseTree(const std::string &key) {
        Node<std::string, std::string> *currentNode = root;

        while (!currentNode->isLeaf) {
            int index = 0;
            while (index < currentNode->keys.size() && key >= currentNode->keys[index]) {
                index++;
            }
            currentNode = currentNode->children[index];
        }

        return currentNode;
    }

    void insertSortedOrder(Node<std::string, std::string> *node,
                         const std::string &key, const std::string &value) {
        auto nextPosition = std::lower_bound(node->keys.begin(), node->keys.end(), key);
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
            if (node->keys.size() <= treeOrder) {
                return {"", nullptr};
            }

            Node<std::string, std::string> *newNode = new Node<std::string, std::string>(true);

            auto keyMidpoint = node->keys.begin() + (node->keys.size() / 2);
            std::move(keyMidpoint, node->keys.end(), std::back_inserter(newNode->keys));
            node->keys.erase(keyMidpoint, node->keys.end());

            auto valuesMidpoint = node->values.begin() + (node->values.size() / 2);
            std::move(valuesMidpoint, node->values.end(), std::back_inserter(newNode->values));
            node->values.erase(valuesMidpoint, node->values.end());

            newNode->nextLeaf = node->nextLeaf;
            node->nextLeaf = newNode;

            return {newNode->keys[0], newNode};
        }

        auto matchingElement = std::find(node->keys.begin(), node->keys.end(), key);
        int index = matchingElement - node->keys.begin();
        auto childRes = recursiveInsert(node->children[index], key, value);

        if (childRes.second == nullptr) {
            return {"", nullptr};
        }

        node->keys.insert(node->keys.begin() + index, childRes.first);
        node->children.insert(node->children.begin() + index + 1, childRes.second);

        if (node->keys.size() <= treeOrder) {
            return {"", nullptr};
        }

        int splitPoint = treeOrder / 2;
        std::string promotedKey = node->keys[splitPoint];

        Node<std::string, std::string> *newNode = new Node<std::string, std::string>(false);
        newNode->keys.assign(node->keys.begin() + splitPoint + 1, node->keys.end());
        newNode->children.assign(node->children.begin() + splitPoint + 1, node->children.end());

        node->keys.erase(node->keys.begin() + splitPoint, node->keys.end());
        node->children.erase(node->children.begin() + splitPoint + 1, node->children.end());

        return {promotedKey, newNode};
    }

    void insert(const std::string &key, const std::string &value) {
        std::pair<std::string, Node<std::string, std::string> *> result =
            recursiveInsert(root, key, value);

        if (result.second != nullptr) {
            Node<std::string, std::string> *newRoot = new Node<std::string, std::string>(false);
            newRoot->children.push_back(root);
            newRoot->keys.push_back(result.first);
            newRoot->children.push_back(result.second);
            root = newRoot;
        }
    }

    std::optional<std::string> search(const std::string &key) {
        Node<std::string, std::string> *leafNode = traverseTree(key);

        auto matchingElement = std::find(leafNode->keys.begin(), leafNode->keys.end(), key);
        if (matchingElement == leafNode->keys.end()) {
            return std::nullopt;
        }

        int index = matchingElement - leafNode->keys.begin();
        return leafNode->values[index];
    }

    void remove(const std::string &key) {
        // if the tree has not been made yet cant delete anything
        if (root == nullptr) return;


    }
};