#include <iostream>
#include "B_tree.h"
using namespace std;
int main() {
    B_tree tree = B_tree();
    tree.insert(1, 10);
    tree.insert(5, 50);
    tree.insert(2, 250);
    tree.insert(10, 120);
    tree.insert(51, 510);
    tree.insert(23, 150);
    tree.insert(253, 150);
    tree.insert(123, 1510);
    tree.insert(3, 120);
    tree.insert(500, 1);
    tree.traverse();
    tree.print_tree();
    return 0;
}
