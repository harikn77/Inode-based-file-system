#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
using namespace std;
const int DISK_SIZE = 256 * 1024; // 256KB
const int BLOCK_SIZE = 4096;      // 4KB
const int INODE_SIZE = 256;       // 256B
const int NUM_OF_INODE_BLOCKS = 5;
const int NUM_OF_INODES = NUM_OF_INODE_BLOCKS * (BLOCK_SIZE / INODE_SIZE);
const int NUM_OF_DATA_BLOCKS = DISK_SIZE / BLOCK_SIZE;

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

map<string, int> files;
char inode_bitmap[NUM_OF_INODES];
char data_bitmap[NUM_OF_DATA_BLOCKS];
superblock sb;

void create_disk(string disk_name) {
  ofstream disk(disk_name, ios::out | ios::binary);
  superblock sb;
  sb.nblocks = NUM_OF_DATA_BLOCKS;
  sb.ninodes = NUM_OF_INODES;
  disk.write((char *)&sb, sizeof(superblock));
  disk.seekp(BLOCK_SIZE, ios::beg);
  string inode_bitmap = string(NUM_OF_INODES, '0');
  string data_bitmap = string(NUM_OF_DATA_BLOCKS, '0');
  disk.write(inode_bitmap.c_str(), BLOCK_SIZE);
  disk.seekp(2 * BLOCK_SIZE, ios::beg);
  disk.write(data_bitmap.c_str(), BLOCK_SIZE);
  disk.seekp(DISK_SIZE - 1, ios::beg);
  disk.write("\0", 1);
  disk.close();
}
bool mount_disk(fstream &disk) {
  disk.read((char *)&sb, sizeof(superblock));
  cout << "superblock: " << endl;
  cout << "nblocks: " << sb.nblocks << endl;
  cout << "ninodes: " << sb.ninodes << endl;
  disk.seekg(BLOCK_SIZE, ios::beg);

  disk.read(inode_bitmap, NUM_OF_INODES);
  cout << "inode bitmap: " << endl;
  for (int i = 0; i < NUM_OF_INODES; i++) {
    cout << inode_bitmap[i];
  }
  cout << endl;
  disk.seekg(2 * BLOCK_SIZE, ios::beg);
  disk.read(data_bitmap, NUM_OF_DATA_BLOCKS);
  cout << "data bitmap: " << endl;
  for (int i = 0; i < NUM_OF_DATA_BLOCKS; i++) {
    cout << data_bitmap[i];
  }
  cout << endl;
  return true;
}
bool disk_mounted = false;
int main() {
start:
  while (true) {
    cout << "1. create disk\n";
    cout << "2. mount disk\n";
    cout << "3. exit\n";
    int choice;
    cout << "Enter your choice: ";
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
      fstream disk(name, ios::in | ios::binary);
      if (!disk) {
        cout << "Disk not found" << endl;
        continue;
      } else {
        mount_disk(disk);
        disk_mounted = true;
        cout << "disk mounted\n";
        while (disk_mounted) {
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
          cout << "Enter your choice: ";
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
          case 10: {
            cout << "disk unmounted\n";
            disk.close();
            disk_mounted = false;
            goto start;
          }
          }
        }
      }
    }
    case 3:
      return 0;
    }
  }
}