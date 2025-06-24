#include "B_tree.h"

long B_tree::allocateNode(BTreeNode& node)
{
    fstream file("data/nodes.dat", ios::in | ios::out | ios::binary);

    if (!file.is_open()) {
        file.open("data/nodes.dat", ios::out | ios::binary);
        file.close();
        file.open("data/nodes.dat", ios::in | ios::out | ios::binary);
    }

    if (freeListHead != -1) {
        file.seekg(freeListHead);
        BTreeNode freeNode;
        file.read(reinterpret_cast<char*>(&freeNode), sizeof(BTreeNode));

        long reusedOffset = freeListHead;
        freeListHead = freeNode.children[0]; 

        node.selfOffset = reusedOffset;

        file.seekp(reusedOffset);
        file.write(reinterpret_cast<char*>(&node), sizeof(BTreeNode));
        file.close();
#ifdef MESSAGE
        cout << "Reused node at offset: " << reusedOffset << endl;
#endif
        return reusedOffset;
    }
    else {
        file.seekp(0, ios::end);
        long offset = file.tellp();
        node.selfOffset = offset;

        file.write(reinterpret_cast<char*>(&node), sizeof(BTreeNode));
        file.close();
#ifdef MESSAGE
        cout << "Created new node at offset: " << offset << endl;
#endif
        return offset;
    }
}

void B_tree::disk_write(BTreeNode& node) {
    if (node.selfOffset == -1) {
#if defined(MESSAGE) || defined(FIX)
        cerr << "Error: trying to write node without selfOffset\n";
#endif
        return;
    }

    std::fstream file("data/nodes.dat", std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
#if defined(MESSAGE) || defined(FIX)
        cerr << "Error: unable to open data/nodes.dat for writing\n" << endl;
#endif
        return;
    }

    file.seekp(node.selfOffset);
    file.write(reinterpret_cast<char*>(&node), sizeof(BTreeNode));
    file.close();
}


BTreeNode B_tree::disk_read(long offset)
{
    BTreeNode node;
    ifstream file("data/nodes.dat", std::ios::binary);
    if (!file.is_open()) {
#if defined(MESSAGE) || defined(FIX)
        cerr << "Error: Failed to open data/nodes.dat file for reading.\n" << endl;
#endif
        return node;
    }

    file.seekg(0, std::ios::end);
    long fileSize = file.tellg();

    if (offset < 0 || offset + sizeof(BTreeNode) > fileSize) {
#if defined(MESSAGE) || defined(FIX)
        cerr << "Error: Invalid offset " << offset <<" for reading node.\n" << endl;
#endif
        file.close();
        return node;
    }

    file.seekg(offset);
    file.read(reinterpret_cast<char*>(&node), sizeof(BTreeNode));

    if (!file) {
#if defined(MESSAGE) || defined(FIX)
       cerr << "Error: Failed to read node from file to position " << offset << ".\n";
#endif
    }

    file.close();
    return node;
}

int B_tree::search(int key, const BTreeNode& node)
{
    int i = 0;
    while (i < node.numKeys && key > node.keys[i])
        i++;

    if (i < node.numKeys && key == node.keys[i])
        return node.values[i];

    if (node.leaf)
        return -1;

    // Перевірка валідності children[i]
    if (node.children[i] == -1) {
#if defined(MESSAGE) || defined(FIX)
        cerr << "Error: Child node missing (children [" << i << "] == -1)\n";
#endif
        return -1;
    }

    BTreeNode next_node = disk_read(node.children[i]);

    // Чи вузол порожній (наприклад, selfOffset == -1)
    if (next_node.selfOffset == -1) {
#if defined(MESSAGE) || defined(FIX)
        cerr << "Error: Failed to read child node at position " << node.children[i] << "\n";
#endif
        return -1;
    }

    return search(key, next_node);
}

void B_tree::split_child(BTreeNode& x, int i)
{
    BTreeNode y = disk_read(x.children[i]);
    if (y.numKeys != MAX_KEYS) {
#if defined(MESSAGE) || defined(FIX)
        cerr << "Error: Cannot split non-full node.\n";
#endif
        return;
    }
    BTreeNode z(y.leaf);
    allocateNode(z);

    z.numKeys = T - 1;

    for (int j = 0; j < T - 1; ++j) {
        z.keys[j] = y.keys[j + T];
        z.values[j] = y.values[j + T];
    }

    if (!y.leaf) {
        for (int j = 0; j < T; ++j)
            z.children[j] = y.children[j + T];
    }

    y.numKeys = T - 1;

    for (int j = x.numKeys; j >= i + 1; --j)
        x.children[j + 1] = x.children[j];

    x.children[i + 1] = z.selfOffset;

    for (int j = x.numKeys - 1; j >= i; --j) {
        x.keys[j + 1] = x.keys[j];
        x.values[j + 1] = x.values[j];
    }

    x.keys[i] = y.keys[T - 1];
    x.values[i] = y.values[T - 1];
    x.numKeys++;

    disk_write(x);
    disk_write(y);
    disk_write(z);
}

void B_tree::insert_nonfull(int key, int value, BTreeNode& node)
{
    int i = node.numKeys - 1;
    if (node.leaf) {
        while (i >= 0 && key < node.keys[i]) {
            node.keys[i + 1] = node.keys[i];
            node.values[i + 1] = node.values[i];
            i--;
        }
        node.keys[i + 1] = key;
        node.values[i + 1] = value;
        node.numKeys++;
        disk_write(node);
        size++;
#ifdef MESSAGE
        cout << "Added key: " << key << ", value: " << value << " to leaf node at offset " << node.selfOffset << endl;
#endif
    }
    else {
        while (i >= 0 && key < node.keys[i]) {
            i--;
        }
        i++;
        BTreeNode next_node = disk_read(node.children[i]);
        if (next_node.selfOffset == -1) {
#if defined(MESSAGE) || defined(FIX)
            cerr << "Error: Failed to read child node at offset " << node.children[i] << endl;
#endif
            return;
        }
        if (next_node.numKeys == MAX_KEYS) {
            split_child(node, i);
            next_node = disk_read(node.children[i]);
            if (key > node.keys[i]) {
                i++;
                next_node = disk_read(node.children[i]);
            }
        }
        insert_nonfull(key, value, next_node);
    }
}

int B_tree::erase(int key)
{
    BTreeNode root = disk_read(rootOffset);
    int result = erase(key, root);

    // Перевірка після видалення
    if (root.numKeys == 0 && !root.leaf) {
        // Корінь став порожнім і має одного нащадка
        BTreeNode newRoot = disk_read(root.children[0]);
        rootOffset = newRoot.selfOffset;
        save_metadata();
#ifdef MESSAGE
        cout << "Root was empty. Promoted child at offset " << rootOffset << " as new root.\n";
#endif
    }
    return result;
}

void B_tree::insert(int key, int value)
{
    if (search(key) != -1) {
#ifdef MESSAGE
        cout << "Key " << key << " already exists. Insert skipped." << endl;
#endif
        return;  
    }
    BTreeNode root = disk_read(rootOffset);
    if (root.numKeys == MAX_KEYS) {
        BTreeNode new_root(false);
        rootOffset = allocateNode(new_root);
        new_root.children[0] = root.selfOffset;
        split_child(new_root, 0);
        disk_write(new_root);
        insert_nonfull(key, value, new_root);
    }
    else {
        insert_nonfull(key, value, root);
    }
    save_metadata();
}
void B_tree::traverse() {
    BTreeNode root = disk_read(rootOffset);
    traverse_node(root);
}

void B_tree::traverse_node(const BTreeNode& node) {
    for (int i = 0; i < node.numKeys; ++i) {
        if (!node.leaf && node.children[i] != -1) {
            BTreeNode child = disk_read(node.children[i]);
            traverse_node(child);
        }

        cout << node.keys[i] << " : " << node.values[i] << endl;
    }

    if (!node.leaf && node.children[node.numKeys] != -1) {
        BTreeNode child = disk_read(node.children[node.numKeys]);
        traverse_node(child);
    }
}

void B_tree::print_tree()
{
    BTreeNode root = disk_read(rootOffset);
    print_node(root, 0);
}

void B_tree::print_node(const BTreeNode& node, int depth)
{
    string indent(depth * 4, ' ');

    cout << indent << "Node at offset: " << node.selfOffset
        << (node.leaf ? " [leaf]" : " [internal]") << endl;

    for (int i = 0; i < node.numKeys; ++i) {
        cout << indent << "  Key: " << node.keys[i]
            << ", Value: " << node.values[i] << endl;
    }

    if (!node.leaf) {
        for (int i = 0; i <= node.numKeys; ++i) {
            if (node.children[i] != -1) {
                BTreeNode child = disk_read(node.children[i]);
                print_node(child, depth + 1);
            }
        }
    }
}

B_tree::B_tree() {
    load_metadata();
    if (rootOffset == -1) {
        BtreeCreate();
        save_metadata();  
    }
}

void B_tree::save_metadata() {
    ofstream file("data/meta.dat", ios::binary);
    if (!file.is_open()) {
#if defined(MESSAGE) || defined(FIX)
        cerr << "Error: Could not open meta.dat to save metadata.\n";
#endif
        return;
    }

    file.write(reinterpret_cast<char*>(&rootOffset), sizeof(rootOffset));
    file.write(reinterpret_cast<char*>(&freeListHead), sizeof(freeListHead));
    file.write(reinterpret_cast<char*>(&size), sizeof(size));
    file.close();
}

void B_tree::load_metadata() {
    ifstream file("data/meta.dat", ios::binary);
    if (!file.is_open()) {
        rootOffset = -1;
        freeListHead = -1;
        size = 0;
        return;
    }

    file.read(reinterpret_cast<char*>(&rootOffset), sizeof(rootOffset));
    file.read(reinterpret_cast<char*>(&freeListHead), sizeof(freeListHead));
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    file.close();
#ifdef MESSAGE
    cout << "Metadata loaded: rootOffset = " << rootOffset
        << ", freeListHead = " << freeListHead
        << ", size = " << size << endl;
#endif
}

int B_tree::erase(int key, BTreeNode node) {
    int i = 0;
    while (i < node.numKeys && key > node.keys[i])
        i++;

    if (i < node.numKeys && key == node.keys[i]) {
        int value = node.values[i];
        if (node.leaf) {
            for (int j = i; j < node.numKeys - 1; ++j) {
                node.keys[j] = node.keys[j + 1];
                node.values[j] = node.values[j + 1];
            }
            node.numKeys--;
            size--;
            disk_write(node);
#ifdef MESSAGE
            cout << "Deleted key " << key << " from leaf node.\n";
#endif
            return value;
        }
        else {
            BTreeNode pred = disk_read(node.children[i]);
            if (pred.numKeys >= T) {
                node.keys[i] = pred.keys[pred.numKeys - 1];
                node.values[i] = pred.values[pred.numKeys - 1];
                disk_write(node);
                return erase(node.keys[i], disk_read(node.children[i]));
        }

            BTreeNode succ = disk_read(node.children[i + 1]);
            if (succ.numKeys >= T) {
                node.keys[i] = succ.keys[0];
                node.values[i] = succ.values[0];
                disk_write(node);
                return erase(node.keys[i], disk_read(node.children[i + 1]));
            }

            BTreeNode left = disk_read(node.children[i]);
            BTreeNode right = disk_read(node.children[i + 1]);

            left.keys[left.numKeys] = node.keys[i];
            left.values[left.numKeys] = node.values[i];
            for (int j = 0; j < right.numKeys; ++j) {
                left.keys[left.numKeys + 1 + j] = right.keys[j];
                left.values[left.numKeys + 1 + j] = right.values[j];
            } 
            if (!left.leaf) {
                for (int j = 0; j <= right.numKeys; ++j)
                    left.children[left.numKeys + 1 + j] = right.children[j];
            }

            left.numKeys += right.numKeys + 1;
            deleteNode(right);

            for (int j = i + 1; j < node.numKeys; ++j) {
                node.keys[j - 1] = node.keys[j];
                node.values[j - 1] = node.values[j];
                node.children[j] = node.children[j + 1];
            }
            node.numKeys--;

            disk_write(left);
            disk_write(node);
            return erase(key, left);
    }
}
    else {
        if (node.leaf)
            return -1;

        BTreeNode child = disk_read(node.children[i]);

        if (child.numKeys < T) {
            BTreeNode sibling;
            if (i > 0) {
                sibling = disk_read(node.children[i - 1]);
                if (sibling.numKeys >= T) {
                    for (int j = child.numKeys; j > 0; --j) {
                        child.keys[j] = child.keys[j - 1];
                        child.values[j] = child.values[j - 1];
                    }
                    if (!child.leaf) {
                        for (int j = child.numKeys + 1; j > 0; --j)
                            child.children[j] = child.children[j - 1];
                    }

                    child.keys[0] = node.keys[i - 1];
                    child.values[0] = node.values[i - 1];
                    if (!child.leaf)
                        child.children[0] = sibling.children[sibling.numKeys];

                    node.keys[i - 1] = sibling.keys[sibling.numKeys - 1];
                    node.values[i - 1] = sibling.values[sibling.numKeys - 1];
                    sibling.numKeys--;
                    child.numKeys++;

                    disk_write(sibling);
                    disk_write(child);
                    disk_write(node);
                }
                else {
                    sibling = disk_read(node.children[i - 1]);
                    sibling.keys[sibling.numKeys] = node.keys[i - 1];
                    sibling.values[sibling.numKeys] = node.values[i - 1];
                    for (int j = 0; j < child.numKeys; ++j) {
                        sibling.keys[sibling.numKeys + 1 + j] = child.keys[j];
                        sibling.values[sibling.numKeys + 1 + j] = child.values[j];
                    }
                    if (!child.leaf) {
                        for (int j = 0; j <= child.numKeys; ++j)
                            sibling.children[sibling.numKeys + 1 + j] = child.children[j];
                    }

                    sibling.numKeys += child.numKeys + 1;
                    deleteNode(child);

                    for (int j = i; j < node.numKeys; ++j) {
                        node.keys[j - 1] = node.keys[j];
                        node.values[j - 1] = node.values[j];
                        node.children[j] = node.children[j + 1];
                    }
                    node.numKeys--;
                    disk_write(sibling);
                    disk_write(node);
                    child = sibling;
                    i--; 
                }
            }
            else if (i < node.numKeys) {
                sibling = disk_read(node.children[i + 1]);
                if (sibling.numKeys >= T) {
                    child.keys[child.numKeys] = node.keys[i];
                    child.values[child.numKeys] = node.values[i];
                    if (!child.leaf)
                        child.children[child.numKeys + 1] = sibling.children[0];

                    node.keys[i] = sibling.keys[0];
                    node.values[i] = sibling.values[0];
                    for (int j = 1; j < sibling.numKeys; ++j) {
                        sibling.keys[j - 1] = sibling.keys[j];
                        sibling.values[j - 1] = sibling.values[j];
                    }
                    if (!sibling.leaf) {
                        for (int j = 1; j <= sibling.numKeys; ++j)
                            sibling.children[j - 1] = sibling.children[j];
                    }

                    sibling.numKeys--;
                    child.numKeys++;

                    disk_write(sibling);
                    disk_write(child);
                    disk_write(node);
                }
                else {
                    child.keys[child.numKeys] = node.keys[i];
                    child.values[child.numKeys] = node.values[i];
                    for (int j = 0; j < sibling.numKeys; ++j) {
                        child.keys[child.numKeys + 1 + j] = sibling.keys[j];
                        child.values[child.numKeys + 1 + j] = sibling.values[j];
                    }
                    if (!child.leaf) {
                        for (int j = 0; j <= sibling.numKeys; ++j)
                            child.children[child.numKeys + 1 + j] = sibling.children[j];
                    }

                    child.numKeys += sibling.numKeys + 1;
                    deleteNode(sibling);

                    for (int j = i + 1; j < node.numKeys; ++j) {
                        node.keys[j - 1] = node.keys[j];
                        node.values[j - 1] = node.values[j];
                        node.children[j] = node.children[j + 1];
                    }
                    node.numKeys--;
                    disk_write(child);
                    disk_write(node);
                }
            }
        }

        return erase(key, child);
    }
}


void B_tree::deleteNode(BTreeNode& node)
{
    node.numKeys = 0;
    node.children[0] = freeListHead;
    freeListHead = node.selfOffset;
    disk_write(node);
}
