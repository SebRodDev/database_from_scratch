#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

template <typename K, typename V>
struct Node {
    bool isLeaf;
    std::vector<K> keys;
    std::vector<V> values;
    std::vector<Node<K, V>*> children;
    Node<K, V>* nextLeaf;

    Node(bool leaf) : isLeaf(leaf), nextLeaf(nullptr) {}
    Node() : isLeaf(true), nextLeaf(nullptr) {}
};

class BPlusTree {
private:
    using TreeNode = Node<std::string, std::string>;

    TreeNode* root;
    int treeOrder;

    // Recursively delete every node in the tree.
    void deleteTree(TreeNode* node) {
        if (node == nullptr) {
            return;
        }

        if (!node->isLeaf) {
            for (TreeNode* child : node->children) {
                deleteTree(child);
            }
        }

        delete node;
    }

    // Rebuild the separator keys of an internal node.
    //
    // In a B+ tree, node->keys[i] is the smallest key
    // contained in node->children[i + 1].
    void updateKeys(TreeNode* node) {
        if (node->isLeaf) {
            return;
        }

        node->keys.clear();

        for (size_t i = 1; i < node->children.size(); i++) {
            if (!node->children[i]->keys.empty()) {
                node->keys.push_back(node->children[i]->keys[0]);
            }
        }
    }

    // Find which child should contain key.
    //
    // Example:
    // keys = [B, D, F]
    //
    // children:
    //   0: keys < B
    //   1: B <= keys < D
    //   2: D <= keys < F
    //   3: keys >= F
    int getChildIndex(TreeNode* node, const std::string& key) {
        return std::upper_bound(
            node->keys.begin(),
            node->keys.end(),
            key
        ) - node->keys.begin();
    }

    void insertSortedOrder(
        TreeNode* node,
        const std::string& key,
        const std::string& value
    ) {
        auto position = std::lower_bound(
            node->keys.begin(),
            node->keys.end(),
            key
        );

        int index = position - node->keys.begin();

        // If the key already exists, update its value.
        if (position != node->keys.end() && *position == key) {
            node->values[index] = value;
            return;
        }

        node->keys.insert(position, key);
        node->values.insert(
            node->values.begin() + index,
            value
        );
    }

    std::pair<std::string, TreeNode*> recursiveInsert(
        TreeNode* node,
        const std::string& key,
        const std::string& value
    ) {

        // LEAF
        if (node->isLeaf) {
            insertSortedOrder(node, key, value);

            // No split required.
            if (node->keys.size() <= static_cast<size_t>(treeOrder)) {
                return {"", nullptr};
            }

            // Split leaf.
            TreeNode* newNode = new TreeNode(true);

            size_t midpoint = node->keys.size() / 2;

            newNode->keys.assign(
                node->keys.begin() + midpoint,
                node->keys.end()
            );

            newNode->values.assign(
                node->values.begin() + midpoint,
                node->values.end()
            );

            node->keys.erase(
                node->keys.begin() + midpoint,
                node->keys.end()
            );

            node->values.erase(
                node->values.begin() + midpoint,
                node->values.end()
            );

            // Maintain linked list of leaves.
            newNode->nextLeaf = node->nextLeaf;
            node->nextLeaf = newNode;

            // The first key of the new right leaf gets
            // promoted to the parent.
            return {newNode->keys[0], newNode};
        }

        // INTERNAL NODE
        int index = getChildIndex(node, key);

        auto childResult = recursiveInsert(
            node->children[index],
            key,
            value
        );

        // Child didn't split.
        if (childResult.second == nullptr) {
            updateKeys(node);
            return {"", nullptr};
        }

        // Child split, so insert the new child.
        node->children.insert(
            node->children.begin() + index + 1,
            childResult.second
        );

        updateKeys(node);

        // Internal node doesn't need splitting.
        if (node->keys.size() <= static_cast<size_t>(treeOrder)) {
            return {"", nullptr};
        }

        // SPLIT INTERNAL NODE
        size_t splitPoint = node->keys.size() / 2;

        std::string promotedKey = node->keys[splitPoint];

        TreeNode* newNode = new TreeNode(false);

        // Keys after promotedKey go into the new node.
        newNode->keys.assign(
            node->keys.begin() + splitPoint + 1,
            node->keys.end()
        );

        // Children corresponding to those keys go into new node.
        newNode->children.assign(
            node->children.begin() + splitPoint + 1,
            node->children.end()
        );

        // Left node keeps keys before promotedKey.
        node->keys.erase(
            node->keys.begin() + splitPoint,
            node->keys.end()
        );

        // Left node keeps children through splitPoint.
        node->children.erase(
            node->children.begin() + splitPoint + 1,
            node->children.end()
        );

        return {promotedKey, newNode};
    }

    // Returns true if the node is underfull.
    bool isUnderfull(TreeNode* node) const {
        if (node == root) {
            return false;
        }

        if (node->isLeaf) {
            // Minimum leaf occupancy is ceil(treeOrder / 2).
            size_t minimumKeys =
                (static_cast<size_t>(treeOrder) + 1) / 2;

            return node->keys.size() < minimumKeys;
        }

        // An internal node has max treeOrder + 1 children.
        // Minimum is ceil((treeOrder + 1) / 2).
        size_t minimumChildren =
            (static_cast<size_t>(treeOrder) + 2) / 2;

        return node->children.size() < minimumChildren;
    }

    void borrowFromLeft(
        TreeNode* parent,
        int index
    ) {
        TreeNode* node = parent->children[index];
        TreeNode* leftSibling = parent->children[index - 1];

        if (node->isLeaf) {
            // Move the largest key/value from left sibling.
            node->keys.insert(
                node->keys.begin(),
                leftSibling->keys.back()
            );

            node->values.insert(
                node->values.begin(),
                leftSibling->values.back()
            );

            leftSibling->keys.pop_back();
            leftSibling->values.pop_back();
        }
        else {
            // Move the last child from left sibling.
            TreeNode* child = leftSibling->children.back();

            leftSibling->children.pop_back();

            node->children.insert(
                node->children.begin(),
                child
            );
        }

        updateKeys(leftSibling);
        updateKeys(node);
        updateKeys(parent);
    }

    void borrowFromRight(
        TreeNode* parent,
        int index
    ) {
        TreeNode* node = parent->children[index];
        TreeNode* rightSibling = parent->children[index + 1];

        if (node->isLeaf) {
            // Move the smallest key/value from right sibling.
            node->keys.push_back(rightSibling->keys.front());
            node->values.push_back(rightSibling->values.front());

            rightSibling->keys.erase(
                rightSibling->keys.begin()
            );

            rightSibling->values.erase(
                rightSibling->values.begin()
            );
        }
        else {
            // Move the first child from right sibling.
            TreeNode* child = rightSibling->children.front();

            rightSibling->children.erase(
                rightSibling->children.begin()
            );

            node->children.push_back(child);
        }

        updateKeys(node);
        updateKeys(rightSibling);
        updateKeys(parent);
    }

    void mergeWithLeft(
        TreeNode* parent,
        int index
    ) {
        TreeNode* node = parent->children[index];
        TreeNode* leftSibling = parent->children[index - 1];

        if (node->isLeaf) {
            leftSibling->keys.insert(
                leftSibling->keys.end(),
                node->keys.begin(),
                node->keys.end()
            );

            leftSibling->values.insert(
                leftSibling->values.end(),
                node->values.begin(),
                node->values.end()
            );

            // Preserve leaf linked list.
            leftSibling->nextLeaf = node->nextLeaf;
        }
        else {
            leftSibling->children.insert(
                leftSibling->children.end(),
                node->children.begin(),
                node->children.end()
            );
        }

        delete node;

        parent->children.erase(
            parent->children.begin() + index
        );

        updateKeys(leftSibling);
        updateKeys(parent);
    }

    void mergeWithRight(
        TreeNode* parent,
        int index
    ) {
        TreeNode* node = parent->children[index];
        TreeNode* rightSibling = parent->children[index + 1];

        if (node->isLeaf) {
            node->keys.insert(
                node->keys.end(),
                rightSibling->keys.begin(),
                rightSibling->keys.end()
            );

            node->values.insert(
                node->values.end(),
                rightSibling->values.begin(),
                rightSibling->values.end()
            );

            node->nextLeaf = rightSibling->nextLeaf;
        }
        else {
            node->children.insert(
                node->children.end(),
                rightSibling->children.begin(),
                rightSibling->children.end()
            );
        }

        delete rightSibling;

        parent->children.erase(
            parent->children.begin() + index + 1
        );

        updateKeys(node);
        updateKeys(parent);
    }

    void rebalanceChild(
        TreeNode* parent,
        int index
    ) {
        TreeNode* child = parent->children[index];

        if (!isUnderfull(child)) {
            return;
        }

        // Try borrowing from left sibling.
        if (index > 0) {
            TreeNode* leftSibling = parent->children[index - 1];

            bool canBorrow;

            if (child->isLeaf) {
                size_t minimumKeys =
                    (static_cast<size_t>(treeOrder) + 1) / 2;

                canBorrow =
                    leftSibling->keys.size() > minimumKeys;
            }
            else {
                size_t minimumChildren =
                    (static_cast<size_t>(treeOrder) + 2) / 2;

                canBorrow =
                    leftSibling->children.size() > minimumChildren;
            }

            if (canBorrow) {
                borrowFromLeft(parent, index);
                return;
            }
        }

        // Try borrowing from right sibling.
        if (index + 1 < static_cast<int>(parent->children.size())) {
            TreeNode* rightSibling = parent->children[index + 1];

            bool canBorrow;

            if (child->isLeaf) {
                size_t minimumKeys =
                    (static_cast<size_t>(treeOrder) + 1) / 2;

                canBorrow =
                    rightSibling->keys.size() > minimumKeys;
            }
            else {
                size_t minimumChildren =
                    (static_cast<size_t>(treeOrder) + 2) / 2;

                canBorrow =
                    rightSibling->children.size() > minimumChildren;
            }

            if (canBorrow) {
                borrowFromRight(parent, index);
                return;
            }
        }

        // Couldn't borrow, so merge.
        if (index > 0) {
            mergeWithLeft(parent, index);
        }
        else if (index + 1 < static_cast<int>(parent->children.size())) {
            mergeWithRight(parent, index);
        }
    }

    void removeInternal(
        TreeNode* node,
        const std::string& key
    ) {
        // ------------------------------------------------------------
        // LEAF
        // ------------------------------------------------------------
        if (node->isLeaf) {
            auto it = std::lower_bound(
                node->keys.begin(),
                node->keys.end(),
                key
            );

            if (it == node->keys.end() || *it != key) {
                return;
            }

            int index = it - node->keys.begin();

            node->keys.erase(it);

            node->values.erase(
                node->values.begin() + index
            );

            return;
        }

        // ------------------------------------------------------------
        // INTERNAL NODE
        // ------------------------------------------------------------

        int index = getChildIndex(node, key);

        TreeNode* child = node->children[index];

        removeInternal(child, key);

        // Rebalance if necessary.
        rebalanceChild(node, index);

        // Recalculate separators.
        updateKeys(node);
    }

public:
    explicit BPlusTree(int treeOrder)
        : root(new TreeNode(true)),
        treeOrder(treeOrder) {
        if (treeOrder < 3) {
            throw std::invalid_argument(
                "B+ tree order must be at least 3"
            );
        }
    }

    ~BPlusTree() {
        deleteTree(root);
    }

    TreeNode* traverseTree(const std::string& key) {
        TreeNode* currentNode = root;

        while (!currentNode->isLeaf) {
            int index = getChildIndex(currentNode, key);
            currentNode = currentNode->children[index];
        }

        return currentNode;
    }

    void insert(
        const std::string& key,
        const std::string& value
    ) {
        auto result = recursiveInsert(
            root,
            key,
            value
        );

        // Root split.
        if (result.second != nullptr) {
            TreeNode* newRoot = new TreeNode(false);

            newRoot->children.push_back(root);
            newRoot->children.push_back(result.second);

            updateKeys(newRoot);

            root = newRoot;
        }
    }

    std::string search(
        const std::string& key
    ) {
        TreeNode* leafNode = traverseTree(key);

        auto it = std::lower_bound(
            leafNode->keys.begin(),
            leafNode->keys.end(),
            key
        );

        if (it == leafNode->keys.end() || *it != key) {
            return std::string();
        }

        int index = it - leafNode->keys.begin();

        return leafNode->values[index];
    }

    void remove(const std::string& key) {
        if (root == nullptr) {
            return;
        }

        removeInternal(root, key);

        // If the root is an internal node with only one child,
        // collapse the root.
        if (!root->isLeaf && root->children.size() == 1) {
            TreeNode* oldRoot = root;

            root = root->children[0];

            oldRoot->children.clear();

            delete oldRoot;
        }
        // The tree can never have an empty internal root.
        // If the root leaf becomes empty, that's still a valid
        // empty B+ tree.
        if (!root->isLeaf) {
            updateKeys(root);
        }
    }
};
