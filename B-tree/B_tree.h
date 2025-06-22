#pragma once
#include <fstream>
#include <iostream>
using namespace std;
const int T = 3;
const int MAX_KEYS = 2 * T - 1;
const int MAX_CHILDREN = 2 * T;
class BTreeNode
{
public:
    bool leaf;
    int numKeys;
    int keys[MAX_KEYS];
    int values[MAX_KEYS];      
    long children[MAX_CHILDREN];
    long selfOffset;
    BTreeNode(bool isLeaf = false) {
        leaf = isLeaf;
        numKeys = 0;
        selfOffset = -1;
        for (int i = 0; i < MAX_KEYS; ++i) {
            keys[i] = -1;
            values[i] = -1;
        }
        for (int i = 0; i < MAX_CHILDREN; ++i)
            children[i] = -1;
    }
};
class B_tree
{
    long rootOffset = -1;      
    long freeListHead = -1;
    int size = 0;
    void BtreeCreate() {
        BTreeNode newRoot(true);
        rootOffset = allocateNode(newRoot);
    }
    long allocateNode(BTreeNode& node);
    void disk_write(BTreeNode& node);
    BTreeNode disk_read(long offset);
    int search(int key, const BTreeNode& node);
public:
    B_tree() {
        BtreeCreate();
    }
    int search(int key) {
        BTreeNode root = disk_read(rootOffset);
        return search(key, root);
    }
};

