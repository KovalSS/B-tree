#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;
int main() {
    // Create directory if it doesn't exist
    string folderName = "SaveData";

    // Create a file outside the folder
    ofstream outsideFile("file_outside.txt");
    if (outsideFile.is_open()) {
        outsideFile << "This file is outside the folder.\n";
        outsideFile.close();
        cout << "File 'file_outside.txt' created successfully.\n";
    }
    else {
        cerr << "Failed to create 'file_outside.txt'.\n";
    }

    // Create a file inside the folder
    ofstream insideFile(folderName + "/file_in_folder.txt");
    if (insideFile.is_open()) {
        insideFile << "This file is inside the folder.\n";
        insideFile.close();
        cout << "File '" << folderName << "/file_in_folder.txt' created successfully.\n";
    }
    else {
        cerr << "Failed to create '" << folderName << "/file_in_folder.txt'.\n";
    }

    return 0;
}
