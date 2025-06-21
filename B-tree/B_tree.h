#pragma once
#include <fstream>
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
    BTreeNode root;
    int size = 0;
    void BtreeCreate() {
        root = BTreeNode(true);
        disk_write(root);
    }
    void disk_write(BTreeNode node);
public:
    B_tree() {
        BtreeCreate();
    }
};

