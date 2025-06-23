#pragma once
#include <fstream>
#include <iostream>
#define FIX
//#define MESSAGE
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
    void split_child(BTreeNode& x, int i);
    void insert_nonfull(int key, int value, BTreeNode& node);
    void traverse_node(const BTreeNode& node); 
    void print_node(const BTreeNode& node, int depth);
    void save_metadata();
    void load_metadata();

public:
    B_tree();
    int search(int key) {
        BTreeNode root = disk_read(rootOffset);
        return search(key, root);
    }
    int pop(int key);
    void insert(int key, int value);
    int getSize() {
        return size;
    }
    void traverse(); 
    void print_tree();
    ~B_tree() {
        save_metadata();
    }

};

