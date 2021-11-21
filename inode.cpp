#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
const int MAX_SIZE = 256 * 1024; // 256KB

int main() {
  while (true) {
    cout << "1.create disk\n";
    cout << "2.mount disk\n";
    cout << "3.exit\n";
    int choice;
    cin >> choice;
    switch (choice) {
    case 1: {
      cout << "enter the name of the disk: ";
      string name;
      cin >> name;
      ofstream disk(name, ios::out | ios::binary);
      disk.seekp(MAX_SIZE - 1);
      disk.put('\0');
      disk.close();
      break;
    }
    case 2: {
      cout << "enter the name of the disk: ";
      string name;
      cin >> name;
      ifstream disk(name, ios::in | ios::binary);
      if (!disk) {
        cout << "disk not found\n";
        continue;
      }
      break;
    }
    case 3:
      return 0;
    }
  }
}