#include <iostream>
#include <exception>
#include <algorithm>
#include <queue>
#include <cstring>
#include <functional>
#include "gtest/gtest.h"

using namespace std;

template<typename T>
class BSTNode{
public:
    BSTNode<T>* left{nullptr};
    BSTNode<T>* right{nullptr};
    T data{};
    size_t height{0};
    void insert(T value, BSTNode<T>* newLeaf);
    void remove(T value, BSTNode<T>*& parent);
    void postOrder(vector<T>& values) const;
    void preOrder(vector<T>& values) const;
    void inOrder(vector<T>& values) const;
    void rotLeft();
    void rotRight();
    void balance();
};

template<typename T>
class BSTNodeandParents{
public:
    BSTNode<T>* node{nullptr};
    BSTNode<T>* parent{nullptr};
};

template<typename T>
class BSTree{
private:
    BSTNode<T>* root;
    int treeSize;
public:
   BSTree();
   ~BSTree();
   bool includes(T value);
   void insert(T value);
   void remove(T value);
   vector<T> inOrder() const;
   vector<T> preOrder() const;
   vector<T> postOrder() const;
   int size() const;
   bool isEmpty() const;
private:
    BSTNodeandParents<T> searchSubtree(T value, BSTNode<T>* root, BSTNode<T>* parent);
};

template<typename T>
void BSTNode<T>::insert(T value, BSTNode<T>* newLeaf){
    //go right
    if (right != nullptr && data < value){
        right->insert(value, newLeaf);
    }
    //go left
    else if (left != nullptr && data > value){
        left->insert(value, newLeaf);
    }
    //create right
    else if ( data < value){
        right = newLeaf;
        newLeaf->data = value;
    }
    //create left
    else {
        left = newLeaf;
        newLeaf->data = value;
    }
    balance();
}

// usage of 'parent' is different between two methods, so be careful

template<typename T>
void BSTNode<T>::remove(T value, BSTNode<T>*& self){
    if(value == data){
        if (left == nullptr && right == nullptr){ // 0 children
            self = nullptr;
            delete this;
        } else if (left != nullptr && right != nullptr){ // 2 children
            BSTNode<T>* successor = right;
            BSTNode<T>* successorParent = self;
            while (successor->left != nullptr){
                successorParent = successor;
                successor = successor->left;
            }
            data = successor->data;
            if (this == successorParent){
                right->remove(data, right);
            } else {
                successorParent->remove(data, right);
            }
        }
        else{ // 1 child
            self = (right == nullptr)? left : right;
            delete this;
        }
    }
    else if (value < data && left != nullptr){
        left->remove(value, left);
        balance();
    } else if (value > data && right != nullptr){
        right->remove(value, right);
        balance();
    } 

}

template<typename T>
void BSTNode<T>::rotRight(){
    if (left == nullptr || right == nullptr || left->right == nullptr || left->left == nullptr){
        return;
    }
    BSTNode<T>* P = left;
    BSTNode<T>* A = P->left;
    BSTNode<T>* B = P->right;
    BSTNode<T>* C = right;
    
    T qTemp = data;
    data = P->data;
    P->data = qTemp;
    
    left = A;
    right = P;
    P->right = C;
    P->left = B;
}

template<typename T>
void BSTNode<T>::rotLeft(){
    if (left == nullptr || right == nullptr || right->right == nullptr || right->left == nullptr){
        return;
    }
    BSTNode<T>* Q = right;
    BSTNode<T>* A = left;
    BSTNode<T>* B = Q->right;
    BSTNode<T>* C = Q->right;
    
    T pTemp = data;
    data = Q->data;
    Q->data = pTemp;
    
    left = Q;
    right = C;
    Q->right = B;
    Q->left = A;
}

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
    if (treeSize == 0){
        return ordered;
    }
    root->preOrder(ordered);
    return ordered;
}

template<typename T>
vector<T> BSTree<T>::inOrder() const {
    vector<T> ordered;
    if (treeSize == 0){
        return ordered;
    }
    root->inOrder(ordered);
    return ordered;
}

template<typename T>
BSTree<T>::BSTree()
:root{nullptr},
treeSize{0}{}

template<typename T>
BSTree<T>::~BSTree(){
    vector<T> elements = inOrder();
    for (T element : elements){
        remove(element);
    }
}

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
    if (treeSize == 0){
        return ordered;
    }
    root->postOrder(ordered);
    return ordered;
}

template<typename T>
void BSTNode<T>::balance(){
    int rightHeight = (right == nullptr)? -1 : right->height;
    int leftHeight = (left == nullptr)? -1 : left->height;
    height = (leftHeight > rightHeight)? leftHeight + 1 : rightHeight + 1;
    if (rightHeight - leftHeight > 1){ // Right, ____
        rightHeight = (right->right == nullptr)? -1 : right->right->height;
        leftHeight = (right->left == nullptr)? -1 : right->left->height;
        if (rightHeight > leftHeight){ // Right, Right
            rotLeft();
        } else { // Right, Left
            rotRight();
            rotLeft();
        }
    } else if (rightHeight - leftHeight < -1){ // Left, ____
        rightHeight = (left->right == nullptr)? -1 : left->right->height;
        leftHeight = (left->left == nullptr)? -1 : left->left->height;
        if (rightHeight > leftHeight){ // Left, Right
            rotLeft();
            rotRight();
        } else { // Left, Left
            rotRight();
        }
    }
}

template<typename T>
BSTNodeandParents<T> BSTree<T>::searchSubtree(T value, BSTNode<T>* root, BSTNode<T>* parent){
    BSTNodeandParents<T> nodeNParent;
    nodeNParent.parent = parent;
    if (root == nullptr || treeSize == 0){
        return nodeNParent;
    }
    if(root->data == value){
        nodeNParent.node = root;
        return nodeNParent;
    }
    if (root->left != nullptr && root->data > value){
        return searchSubtree(value, root->left, root);
    }
    if (root->right != nullptr && root->data < value){ 
        return searchSubtree(value, root->right, root);
    }
    return nodeNParent;
}

template<typename T>
void BSTree<T>::remove(T value){
    if (!includes(value)){
        return;
    }
    treeSize--;
    root->remove(value, root);
}

template<typename T>
bool BSTree<T>::includes(T value){
    return (searchSubtree(value, root, nullptr).node != nullptr);
}

template<typename T>
void BSTree<T>::insert(T value){
    BSTNode<T>* newLeaf = new BSTNode<T>;
    treeSize++;
    if (root == nullptr){
        newLeaf->data = value;
        root = newLeaf;
        return;
    } else if (includes(value)){
        delete newLeaf;
        treeSize--;
        return;
    }
    root->insert(value, newLeaf);
}

template<typename T>
void assertIncludesAll(BSTree<T>& tree, vector<T> values){
    for (T element : values){
        ASSERT_TRUE(tree.includes(element));
    }
}

template<typename T>
void assertIncludesExactly(BSTree<T>& tree, vector<T> values){
    vector<T> traversed = tree.postOrder();
    std::sort(values.begin(), values.end());
    std::sort(traversed.begin(), traversed.end());
    for (int i = 0; i < values.size(); i++){
        ASSERT_EQ(values[i], traversed[i]);
    }
}

template<typename T>
void insertVector(BSTree<T>& tree, vector<T> values){
    for (T value : values){
        tree.insert(value);
    }
}

template<typename T>
void removeVector(BSTree<T>& tree, vector<T> values){
    for (T value : values){
        tree.remove(value);
    }
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


TEST(TestTree, insertFour){
    BSTree<int> tree;
    tree.insert(3);
    tree.insert(1);
    tree.insert(2);
    tree.insert(4); 
}

TEST(TestTree, oneElement){
    BSTree<int> tree;
    tree.insert(38);
    ASSERT_TRUE(tree.includes(38));
    ASSERT_EQ(tree.size(), 1);
    ASSERT_FALSE(tree.isEmpty());
}

TEST(TestTree, breaker){
    BSTree<int> tree;
    tree.insert(1);
    tree.insert(3);
    tree.insert(4);
    tree.remove(1);
    
    assertIncludesExactly(tree , vector<int>{3,4});
}

TEST(TestTree, removeRoot){
    BSTree<int> tree = makeSixElementTree();
    ASSERT_TRUE(tree.includes(8));
    ASSERT_EQ(tree.size(), 6);
    tree.remove(8);
    ASSERT_FALSE(tree.includes(8));
    ASSERT_EQ(tree.size(), 5);

    vector<int> remainingVals = {5,2,10,9,7};
    assertIncludesAll(tree, remainingVals);
    assertIncludesExactly(tree, remainingVals);
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
    
    tree.insert(3);
    ASSERT_TRUE(tree.includes(3));
}

TEST(TestTree, removeLeaf){
    BSTree<int> tree = makeSixElementTree();
    ASSERT_TRUE(tree.includes(2));
    ASSERT_EQ(tree.size(), 6);
    tree.remove(2);
    ASSERT_FALSE(tree.includes(2));
    ASSERT_EQ(tree.size(), 5);
    
    vector<int> remainingVals = {8,5,10,9,7};
    assertIncludesAll(tree, remainingVals);
    assertIncludesExactly(tree, remainingVals);
}

TEST(TestTree, removeParentOneChild){
    BSTree<int> tree = makeSixElementTree();
    ASSERT_TRUE(tree.includes(10));
    ASSERT_EQ(tree.size(), 6);
    tree.remove(10);
    ASSERT_FALSE(tree.includes(10));
    ASSERT_EQ(tree.size(), 5);
    
    vector<int> remainingVals = {8,5,2,9,7};
    assertIncludesAll(tree, remainingVals);
    assertIncludesExactly(tree, remainingVals);
}

TEST(TestTree, removeParentTwoChildren){
    BSTree<int> tree = makeSixElementTree();
    ASSERT_TRUE(tree.includes(5));
    ASSERT_EQ(tree.size(), 6);
    tree.remove(5);
    ASSERT_FALSE(tree.includes(5));
    ASSERT_EQ(tree.size(), 5);
    
    vector<int> remainingVals = {8,2,10,9,7};
    assertIncludesAll(tree, remainingVals);
    assertIncludesExactly(tree, remainingVals);
}

TEST(TestTree, postOrderBasic){
    BSTree<int> tree = makeSixElementTree();
    vector<int> sorted = {2,7,5,9,10,8};
    ASSERT_EQ(tree.postOrder(), sorted);
}

TEST(TestTree, preOrderBasic){
    BSTree<int> tree = makeSixElementTree();
    vector<int> sorted = {8,5,2,7,10,9};
    ASSERT_EQ(tree.preOrder(), sorted);
}

TEST(TestTree, inOrderBasic){
    BSTree<int> tree = makeSixElementTree();
    vector<int> sorted = {2,5,7,8,9,10};
    ASSERT_EQ(tree.inOrder(), sorted);
}

TEST(TestTree, breaker2){
    BSTree<int> tree;
    insertVector(tree, vector<int>{3, 25, 26, 26, 20, 17, 27, 5, 9, 23, 23, 29, 28, 13, 30, 7, 23, 25, 7, 2, 7, 8, 29, 28, 21, 29, 8, 17, 12, 20, 5, 27, 29, 12, 19, 26, 29, 28, 17, 11, 10, 6, 27, 16, 20, 4, 14, 9, 3, 23, 10, 23, 17, 10, 17, 22, 21, 2, 8, 21, 11, 11, 13, 21, 14, 14, 28, 21, 21, 28, 24, 23, 26, 29, 22, 5, 27, 3, 13, 10, 8, 17, 28, 23, 20, 11, 21, 7, 7, 18, 5, 14, 20, 15, 29, 16, 10, 30, 16, 1});
    removeVector(tree, vector<int>{10, 12, 16, 16, 9, 5, 2, 19, 16, 10, 12, 18});
    ASSERT_EQ(tree.preOrder().size(), 22);
    tree.remove(11);
    ASSERT_EQ(tree.preOrder().size(), 21);
}

/// crashes nondeterministically :(
TEST(TestTree, inOrderIntegers){
    BSTree<int> tree;
    int start = -3e2;
    int end = 3e2;
    vector<int> treeBuilder;
    for (int i = start; i <= end; i++){
        treeBuilder.push_back(i);
    }
//     size_t seed;
    size_t seed = 42;
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
