#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
const int DISK_SIZE = 256 * 1024; // 256KB
const int BLOCK_SIZE = 4096; // 4KB
const int INODE_SIZE = 256; // 256B
const int NUM_OF_INODE_BLOCKS = 5;
struct superblock {
  int nblocks;
  int ninodes;
};
struct inode {
  string fname;
  int fsize;
  int mode;
  int direct[10];
  int single_indirect;
  int double_indirect;
};
void create_disk(string disk_name){
  ofstream disk(disk_name, ios::out | ios::binary);
  superblock sb;
  sb.nblocks = DISK_SIZE / BLOCK_SIZE;
  sb.ninodes = NUM_OF_INODE_BLOCKS * (BLOCK_SIZE / INODE_SIZE);
  disk.write((char *)&sb, sizeof(superblock));
  disk.seekp(BLOCK_SIZE, ios::beg);
  string inode_bitmap = string(BLOCK_SIZE-1, '0');
  string data_bitmap = string(BLOCK_SIZE-1, '0');
  disk.write(inode_bitmap.c_str(), BLOCK_SIZE);
  disk.seekp(2 * BLOCK_SIZE, ios::beg);
  disk.write(data_bitmap.c_str(), BLOCK_SIZE);
  disk.seekp(DISK_SIZE-1, ios::beg);
  disk.write("\0", 1);
  disk.close();
}
void mount_disk(string disk_name){
  ifstream disk(disk_name, ios::in | ios::binary);
  superblock sb;
  disk.read((char *)&sb, sizeof(superblock));
  cout << "superblock: " << endl;
  cout << "nblocks: " << sb.nblocks << endl;
  cout << "ninodes: " << sb.ninodes << endl;
  disk.seekg(BLOCK_SIZE, ios::beg);
  char inode_bitmap[BLOCK_SIZE];
  disk.read(inode_bitmap, BLOCK_SIZE);
  cout << "inode bitmap: " << endl;
  for (int i = 0; i < BLOCK_SIZE; i++) {
    cout << inode_bitmap[i];
  }
  cout << endl;
  disk.seekg(2 * BLOCK_SIZE, ios::beg);
  char data_bitmap[BLOCK_SIZE];
  disk.read(data_bitmap, BLOCK_SIZE);
  cout << "data bitmap: " << endl;
  for (int i = 0; i < BLOCK_SIZE; i++) {
    cout << data_bitmap[i];
  }
  cout << endl;
  disk.close();
}
int main() {
  while (true) {
    cout << "1. create disk\n";
    cout << "2. mount disk\n";
    cout << "3. exit\n";
    int choice;
    cin >> choice;
    switch (choice) {
    case 1: {
      cout << "enter the name of the disk: ";
      string name;
      cin >> name;
      create_disk(name);
      cout << "disk created\n";
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
      } else {
        cout << "disk mounted\n";
        mount_disk(name);
        while (true) {
          cout << "1. create file\n";
          cout << "2. open file\n";
          cout << "3. read file\n";
          cout << "4. write file\n";
          cout << "5. append file\n";
          cout << "6. close file\n";
          cout << "7. delete file\n";
          cout << "8. list files\n";
          cout << "9. list open files\n";
          cout << "10. unmount\n";
          int choice;
          cin >> choice;
          switch (choice) {
          case 1: {
            cout << "enter the name of the file: ";
            string name;
            cin >> name;
            cout << "Coming soon!\n";
            break;
          }
          case 2: {
            cout << "enter the name of the file: ";
            string name;
            cin >> name;
            // check if file exists
            cout << "mode (r/w/p): ";
            string mode;
            cin >> mode;
            cout << "Coming soon!\n";
          }
          }
        }
        break;
      }
    }
    case 3:
      return 0;
    }
  }
}