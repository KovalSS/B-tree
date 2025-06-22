#include "B_tree.h"

long B_tree::allocateNode(BTreeNode& node)
{
    fstream file("data/nodes.dat", ios::in | ios::out | ios::binary);

    // Якщо файл не відкрився — створимо порожній і відкриємо знову
    if (!file.is_open()) {
        file.open("data/nodes.dat", ios::out | ios::binary);
        file.close();
        file.open("data/nodes.dat", ios::in | ios::out | ios::binary);
    }

    if (freeListHead != -1) {
        // Використовуємо вільне місце з free list
        file.seekg(freeListHead);
        BTreeNode freeNode;
        file.read(reinterpret_cast<char*>(&freeNode), sizeof(BTreeNode));

        long reusedOffset = freeListHead;
        freeListHead = freeNode.children[0]; // припустимо, що вільний список зв’язаний через children[0]

        node.selfOffset = reusedOffset;

        // Записуємо вузол на позицію вільного вузла
        file.seekp(reusedOffset);
        file.write(reinterpret_cast<char*>(&node), sizeof(BTreeNode));
        file.close();

        return reusedOffset;
    }
    else {
        // Записуємо в кінець файлу (новий вузол)
        file.seekp(0, ios::end);
        long offset = file.tellp();
        node.selfOffset = offset;

        file.write(reinterpret_cast<char*>(&node), sizeof(BTreeNode));
        file.close();

        return offset;
    }
}

void B_tree::disk_write(BTreeNode& node) {
    if (node.selfOffset == -1) {
        cerr << "Error: trying to write node without selfOffset\n";
        return;
    }

    std::fstream file("data/nodes.dat", std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        cerr << "Error: unable to open data/nodes.dat for writing\n";
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
        std::cerr << "Error: Failed to open data/nodes.dat file for reading.\n";
        return node;
    }

    file.seekg(0, std::ios::end);
    long fileSize = file.tellg();

    if (offset < 0 || offset + sizeof(BTreeNode) > fileSize) {
        cerr << "Error: Invalid offset " << offset <<" for reading node.\n";
        file.close();
        return node;
    }

    file.seekg(offset);
    file.read(reinterpret_cast<char*>(&node), sizeof(BTreeNode));

    if (!file) {
        std::cerr << "Error: Failed to read node from file to position " << offset << ".\n";
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
        cerr << "Error: Child node missing (children [" << i << "] == -1)\n";
        return -1;
    }

    BTreeNode next_node = disk_read(node.children[i]);

    // Чи вузол порожній (наприклад, selfOffset == -1)
    if (next_node.selfOffset == -1) {
        cerr << "Error: Failed to read child node at position " << node.children[i] << "\n";
        return -1;
    }

    return search(key, next_node);
}