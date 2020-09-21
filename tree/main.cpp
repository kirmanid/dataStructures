#include <iostream>
#include <exception>
#include <queue>
#include <cstring>
#include <functional>
#include "gtest/gtest.h"

using namespace std;

template<typename T>
class BSTNode{
public:
    BSTNode<T>* left;
    BSTNode<T>* right;
    T data;
    void postOrder(vector<T>& values) const;
    void preOrder(vector<T>& values) const;
    void inOrder(vector<T>& values) const;
};

template<typename T>
class BSTNodeAndParent{
public:
    BSTNode<T>* node;
    BSTNode<T>* parent;
};

template<typename T>
class BSTree{
private:
    BSTNode<T>* root;
    int treeSize;
public:
   BSTree();
//    ~BSTree();
   bool includes(T value);
   void insert(T value);
   void remove(T value);
   vector<T> inOrder() const;
   vector<T> preOrder() const;
   vector<T> postOrder() const;
   int size() const;
   bool isEmpty() const;
private:
    BSTNodeAndParent<T> searchSubtree(T value, BSTNode<T>* root, BSTNode<T>* parent, bool returnLeaf);
};


template<typename T>
void BSTNode<T>::preOrder(vector<T>& values) const {
    values.push_back(data);
    if (left != nullptr){
        left->preOrder(values);
    }
    if (right != nullptr){
        right->preOrder(values);
    }
}

template<typename T>
void BSTNode<T>::inOrder(vector<T>& values) const {
    if (left != nullptr){
        left->inOrder(values);
    }
    values.push_back(data);
    if (right != nullptr){
        right->inOrder(values);
    }
}

template<typename T>
vector<T> BSTree<T>::preOrder() const {
    vector<T> ordered;
    root->preOrder(ordered);
    return ordered;
}

template<typename T>
vector<T> BSTree<T>::inOrder() const {
    vector<T> ordered;
    root->inOrder(ordered);
    return ordered;
}

template<typename T>
BSTree<T>::BSTree()
:root{nullptr},
treeSize{0}{}

template<typename T>
int BSTree<T>::size() const {
    return treeSize;
}

template<typename T>
bool BSTree<T>::isEmpty() const {
    return (treeSize == 0);
}

template<typename T>
void BSTNode<T>::postOrder(vector<T>& values) const {
    if (left != nullptr){
        left->postOrder(values);
    }
    if (right != nullptr){
        right->postOrder(values);
    }
    values.push_back(data);
}

template<typename T>
vector<T> BSTree<T>::postOrder() const {
    vector<T> ordered;
    root->postOrder(ordered);
    return ordered;
}

template<typename T>
BSTNodeAndParent<T> BSTree<T>::searchSubtree(T value, BSTNode<T>* root, BSTNode<T>* parent, bool returnLeaf){
    BSTNodeAndParent<T> nodeNParent;
    nodeNParent.parent = parent;
    if(root->data == value){
        nodeNParent.node = root;
        return nodeNParent;
    }
    if (root->left != nullptr && root->data > value){
        return searchSubtree(value, root->left, root, returnLeaf);
    }
    if (root->right != nullptr && root->data < value){ 
        return searchSubtree(value, root->right, root, returnLeaf);
    }
    if (returnLeaf){
        nodeNParent.node = root;
        return nodeNParent;
    }
    nodeNParent.node = nullptr;
    return nodeNParent;
}

template<typename T>
void BSTree<T>::remove(T value){
    BSTNodeAndParent<T> removeAndParent = searchSubtree(value, root, root, false);
    BSTNode<T>* toRemove = removeAndParent.node;
    BSTNode<T>* currentParent = removeAndParent.parent;
    if (toRemove == nullptr){
        return;
    }
    treeSize--;
    size_t children = 0;
    if(toRemove->left != nullptr){
        children++;
    }
    if(toRemove->right != nullptr){
        children++;
    }
    if (children == 0){
        delete toRemove;
        if (currentParent->left == toRemove){
            currentParent-> left = nullptr;
        } else {
            currentParent-> right = nullptr;
        }
    } else if (children == 1){
        BSTNode<T>* child;
        if (toRemove->left != nullptr){
            child = toRemove->left;
        } else {
            child = toRemove->right;
        }
        
        if (currentParent->left->data == value){
            currentParent->left = child;
        }
        else {
            currentParent->right = child;
        }
        delete toRemove;
    } else if (children == 2) {
        BSTNode<T>* nextNode = toRemove->right;
        BSTNode<T>* nextNodeParent = toRemove;
        while(nextNode->left != nullptr){
            nextNodeParent = nextNode;
            nextNode = nextNode->left;
        }
        toRemove->data = nextNode->data;
        if (nextNodeParent->left == nextNode){
            nextNodeParent->left = nullptr;
        } else {
            nextNodeParent->right = nullptr;
        }
        delete nextNode;
    }
}

template<typename T>
bool BSTree<T>::includes(T value){
    return (searchSubtree(value, root, root, false).node != nullptr);
}

template<typename T>
void BSTree<T>::insert(T value){
    treeSize++;
    if (root == nullptr){
        root = new BSTNode<T>;
    }
    BSTNode<T>* leaf = searchSubtree(value, root, root, true).node;
    BSTNode<T>* newLeaf = new BSTNode<T>;
    newLeaf->data = value;
    if (value > leaf->data){
        leaf->right = newLeaf;
    } else {
        leaf->left = newLeaf;
    }
}

TEST(TestTree, oneElement){
    BSTree<int> tree;
    tree.insert(38);
    ASSERT_TRUE(tree.includes(38));
    ASSERT_EQ(tree.size(), 1);
    ASSERT_FALSE(tree.isEmpty());
}

TEST(TestTree, integers){
    BSTree<int> tree;
    int cap = 1e4;
    for (int i = 0; i < cap; i++){
        tree.insert(i);
    }
    // two loops to show that tree stores more than one element
    for (int i = 0; i < cap; i++){
        ASSERT_TRUE(tree.includes(i));
    }
}

TEST(TestTree, removeWorksBasic){
    BSTree<int> tree;
    tree.insert(8);
    tree.insert(7);
    tree.insert(10);
    tree.insert(9);
    
    ASSERT_TRUE(tree.includes(10));
    tree.remove(10);
    ASSERT_FALSE(tree.includes(10));
    tree.remove(8);
    ASSERT_FALSE(tree.includes(8));
    tree.remove(7);
    ASSERT_FALSE(tree.includes(7));
    tree.remove(9);
    ASSERT_FALSE(tree.includes(9));
}

BSTree<int> makeSixElementTree(){
    BSTree<int> tree;
    tree.insert(8);
    tree.insert(5);
    tree.insert(10);
    tree.insert(9);
    tree.insert(7);
    tree.insert(2);
    
    return tree;
}

TEST(TestTree, removeLeaf){
    BSTree<int> tree = makeSixElementTree();
    ASSERT_TRUE(tree.includes(2));
    ASSERT_EQ(tree.size(), 6);
    tree.remove(2);
    ASSERT_FALSE(tree.includes(2));
    ASSERT_EQ(tree.size(), 5);
    // ordering tests here? (Also for all other remove tests)
}

TEST(TestTree, removeParentOneChild){
    BSTree<int> tree = makeSixElementTree();
    ASSERT_TRUE(tree.includes(10));
    ASSERT_EQ(tree.size(), 6);
    tree.remove(10);
    ASSERT_FALSE(tree.includes(10));
    ASSERT_EQ(tree.size(), 5);
}

TEST(TestTree, removeParentTwoChildren){
    BSTree<int> tree = makeSixElementTree();
    ASSERT_TRUE(tree.includes(5));
    ASSERT_EQ(tree.size(), 6);
    tree.remove(5);
    ASSERT_FALSE(tree.includes(5));
    ASSERT_EQ(tree.size(), 5);
}

TEST(TestTree, removeRoot){
    BSTree<int> tree = makeSixElementTree();
    ASSERT_TRUE(tree.includes(8));
    ASSERT_EQ(tree.size(), 8);
    tree.remove(8);
    ASSERT_FALSE(tree.includes(8));
    ASSERT_EQ(tree.size(), 5);
}

TEST(TestTree, postOrderBasic){
    BSTree<int> tree = makeSixElementTree();
    vector<int> sorted = {8,5,2,7,10,9};
    ASSERT_EQ(tree.postOrder(), sorted);
}

TEST(TestTree, preOrderBasic){
    BSTree<int> tree = makeSixElementTree();
    vector<int> sorted = {2,7,5,9,10,8};
    ASSERT_EQ(tree.preOrder(), sorted);
}

TEST(TestTree, inOrderBasic){
    BSTree<int> tree = makeSixElementTree();
    vector<int> sorted = {2,5,7,8,9,10};
    ASSERT_EQ(tree.inOrder(), sorted);
}

TEST(TestTree, inOrderIntegers){
    BSTree<int> tree;
    int start = -3e3;
    int end = 3e3;
    vector<int> treeBuilder;
    for (int i = start; i <= end; i++){
        treeBuilder.push_back(i);
    }
    size_t seed;
    srand(seed);
    std::random_shuffle(treeBuilder.begin(), treeBuilder.end());
    for (int element : treeBuilder){
        tree.insert(element);
    }
    vector<int> inOrder = tree.inOrder();
    for (int i = 0; i < end - start; i++){
        ASSERT_TRUE(inOrder[i] < inOrder[i + 1]);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}