#include "B_tree.h"
#include <vector>
#include <iostream>
using namespace std;

int main() {
    B_tree tree;

    for (int i = 0; i < 50; i++) {
        tree.insert(i, i * 10);
    }
    for (int i = 49; i >= 0; i--) {
        tree.erase(i);
        tree.print_tree();
        cout << endl << "==============================================" << endl;
    }
    return 0;
}
