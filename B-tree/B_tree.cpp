#include "B_tree.h"

void B_tree::disk_write(BTreeNode node)
{
    fstream file("data/nodes.dat", ios::in | ios::out | ios::binary);

    // ���� ���� �� ���� � ���������
    if (!file.is_open()) {
        file.open("data/nodes.dat", std::ios::out | std::ios::binary);
        file.close();
        file.open("data/nodes.dat", std::ios::in | std::ios::out | std::ios::binary);
    }

    // ���� �� ����� ����� (offset �� �� �������)
    if (node.selfOffset == -1) {
        file.seekp(0, ios::end);
        node.selfOffset = file.tellp();
    }

    // ������ ����� �� ���� offset
    file.seekp(node.selfOffset);
    file.write(reinterpret_cast<char*>(&node), sizeof(BTreeNode));
    file.close();
}
