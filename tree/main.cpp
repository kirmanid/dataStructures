#include <iostream>
#include <exception>
#include <queue>
#include <cstring>
#include <functional>
#include "gtest/gtest.h"

template<typename T>
class BSTNode{
public:
    BSTNode<T>* left;
    BSTNode<T>* right;
    T data;
};

template<typename T>
class BST{
private:
    BSTNode<T>* root;
    BSTNode<T>* searchSubtree(T value, BSTNode<T>* root, bool returnLeaf);
public:
    bool checkFor(T value);
    void insert(T value);
};

template<typename T>
BSTNode<T>* BST<T>::searchSubtree(T value, BSTNode<T>* root, bool returnLeaf){
    if(root->data == value){
        return root;
    }
    if (root->left != NULL && root->data > value){
        return searchSubtree(value, root->left, returnLeaf);
    }
    if (root->right != NULL && root->data < value){
        return searchSubtree(value, root->right, returnLeaf);
    }
    if (returnLeaf){
        return root;
    }
    return NULL;
}

template<typename T>
bool BST<T>::checkFor(T value){
    return (searchSubtree(value, root, false) != NULL);
}

template<typename T>
void BST<T>::insert(T value){
    if (root == NULL){
        root = new BSTNode<T>;
    }
    BSTNode<T>* leaf = searchSubtree(value, root, true);
    BSTNode<T>* newLeaf = new BSTNode<T>;
    newLeaf->data = value;
    if (value > leaf->data){
        leaf->right = newLeaf;
    } else {
        leaf->left = newLeaf;
    }
}

TEST(TestTree, oneElement){
    BST<int> tree;
    tree.insert(38);
    ASSERT_TRUE(tree.checkFor(38));
}

TEST(TestTree, integers){
    BST<int> tree;
    int cap = 1e4;
    for (int i = 0; i < cap; i++){
        tree.insert(i);
    }
    for (int i = 0; i < cap; i++){
        ASSERT_TRUE(tree.checkFor(i));
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
