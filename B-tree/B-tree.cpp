#include "B_tree.h"
#include <vector>
#include <iostream>
using namespace std;

int main() {
    B_tree tree;

    vector<pair<int, int>> testData = {
        {10, 100}, {20, 200}, {5, 50}, {6, 60},
        {12, 120}, {30, 300}, {7, 70}, {17, 170}
    };

    cout << "=== Inserting elements ===" << endl;
    for (size_t i = 0; i < testData.size(); ++i) {
        tree.insert(testData[i].first, testData[i].second);
    }

    cout << "\n=== Traversing tree ===" << endl;
    tree.traverse();

    cout << "\n=== Printing tree structure ===" << endl;
    tree.print_tree();

    cout << "\n=== Searching for some keys ===" << endl;
    vector<int> keysToSearch = { 6, 17, 99 };
    for (size_t i = 0; i < keysToSearch.size(); ++i) {
        int result = tree.search(keysToSearch[i]);
        if (result != -1)
            cout << "Found key " << keysToSearch[i] << " with value " << result << endl;
        else
            cout << "Key " << keysToSearch[i] << " not found." << endl;
    }

    cout << "\n=== Erasing some keys ===" << endl;
    vector<int> keysToErase = { 6, 10, 30 };
    for (size_t i = 0; i < keysToErase.size(); ++i) {
        int erasedValue = tree.erase(keysToErase[i]);
        if (erasedValue != -1)
            cout << "Erased key " << keysToErase[i] << ", value was " << erasedValue << endl;
        else
            cout << "Key " << keysToErase[i] << " not found or failed to erase." << endl;
    }

    cout << "\n=== Traversing tree after deletions ===" << endl;
    tree.traverse();

    cout << "\n=== Final tree structure ===" << endl;
    tree.print_tree();

    return 0;
}
