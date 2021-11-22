#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstring>
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
  char fname[100];
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
  ofstream disk(disk_name, ios::out | ios::binary |ios::trunc);
  superblock sb;
  sb.nblocks = NUM_OF_DATA_BLOCKS;
  sb.ninodes = NUM_OF_INODES;
  disk.write((char *)&sb, sizeof(superblock));
  
  // string inode_bitmap = string(NUM_OF_INODES, '0');
  // string data_bitmap = string(NUM_OF_DATA_BLOCKS, '0');
  for(int i=0;i<NUM_OF_INODES;i++)
    inode_bitmap[i] = '0';
  for(int i=0;i<NUM_OF_DATA_BLOCKS;i++)
    data_bitmap[i] = '0';
  disk.seekp(BLOCK_SIZE, ios::beg);
  disk.write(inode_bitmap, NUM_OF_INODES);  
  disk.seekp(2 * BLOCK_SIZE, ios::beg);
  disk.write(data_bitmap, NUM_OF_DATA_BLOCKS);
  disk.seekp(DISK_SIZE, ios::beg);
  disk.write("\0", 1);
  disk.close();
}
void mount_disk(fstream &disk) {
  disk.seekg(0, ios::beg);
  disk.read((char *)&sb, sizeof(superblock));
  cout << "superblock: " << endl;
  cout << "nblocks: " << sb.nblocks << endl;
  cout << "ninodes: " << sb.ninodes << endl;
  disk.seekg(BLOCK_SIZE, ios::beg);

  disk.read(inode_bitmap, NUM_OF_INODES);
  // cout << "inode bitmap: " << endl;
  // for (int i = 0; i < NUM_OF_INODES; i++) {
  //   cout << inode_bitmap[i];
  // }
  cout << endl;
  for (int i = 0; i < NUM_OF_INODES; i++) {
    if (inode_bitmap[i] == '1') {
      disk.seekg(3*BLOCK_SIZE + i * INODE_SIZE, ios::beg);
      inode temp;
      disk.read((char *)&temp, sizeof(inode));
      files[temp.fname] = i;
    }
  }
  
  disk.seekg(2 * BLOCK_SIZE, ios::beg);
  disk.read(data_bitmap, NUM_OF_DATA_BLOCKS);
  
  // cout << "data bitmap: " << endl;
  // for (int i = 0; i < NUM_OF_DATA_BLOCKS; i++) {
  //   cout << data_bitmap[i];
  // }
  cout << endl;
}
void create_file(fstream &disk, string fname) {
  if (files.find(fname) != files.end()) {
    cout << "file already exists" << endl;
    return;
  }
  int inode_index = -1;
  for (int i = 0; i < NUM_OF_INODES; i++) {
    if (inode_bitmap[i] == '0') {
      inode_index = i;
      break;
    }
  }
  if (inode_index == -1) {
    cout << "no more inodes" << endl;
    return;
  }
  int data_index = -1;
  for (int i = 0; i < NUM_OF_DATA_BLOCKS; i++) {
    if (data_bitmap[i] == '0') {
      data_bitmap[i] = '1';
      data_index = i;
      break;
    }
  }
  for(int i=0;i<NUM_OF_DATA_BLOCKS;i++)
    cout<<data_bitmap[i];
  cout << endl;
  if (data_index == -1) {
    cout << "no more data blocks, disk is full" << endl;
    return;
  }
  inode_bitmap[inode_index] = '1';
  disk.seekp(BLOCK_SIZE, ios::beg);
  disk.write(inode_bitmap, NUM_OF_INODES);
  disk.seekp(2 * BLOCK_SIZE, ios::beg);
  disk.write(data_bitmap, NUM_OF_DATA_BLOCKS);
  inode new_inode;
  strcpy(new_inode.fname, fname.c_str());
  new_inode.fsize = 0;
  new_inode.mode = 0;
  for (int i = 0; i < 10; i++) {
    new_inode.direct[i] = -1;
  }
  new_inode.direct[0] = data_index;
  new_inode.single_indirect = -1;
  new_inode.double_indirect = -1;
  disk.seekp((3 * BLOCK_SIZE) + (inode_index * INODE_SIZE), ios::beg);
  disk.write((char *)&new_inode, sizeof(new_inode));
  files[fname] = inode_index;
  cout << "file created" << endl;
}
void read_disk(fstream &disk) {
  disk.seekg(BLOCK_SIZE, ios::beg);
  char temp_inode_bitmap[NUM_OF_INODES];
  disk.read(temp_inode_bitmap, NUM_OF_INODES);
  // cout << "inode bitmap: " << endl;
  // for (int i = 0; i < NUM_OF_INODES; i++) {
  //   cout << temp_inode_bitmap[i];
  // }
  cout << endl;
  disk.seekg(2 * BLOCK_SIZE, ios::beg);
  char temp_data_bitmap[NUM_OF_DATA_BLOCKS];
  disk.read(temp_data_bitmap, NUM_OF_DATA_BLOCKS);
  // cout << "data bitmap: " << endl;
  // for (int i = 0; i < NUM_OF_DATA_BLOCKS; i++) {
  //   cout << temp_data_bitmap[i];
  // }
  // cout << endl;

  cout << "files: ";
  for (int i = 0; i < NUM_OF_INODES; i++) {
    if (temp_inode_bitmap[i] == '1') {
      disk.seekg(3 * BLOCK_SIZE + i * INODE_SIZE, ios::beg);
      inode temp;
      disk.read((char *)&temp, sizeof(inode));
      cout << temp.fname << ", ";
    }
  }
  cout << endl;
}
int main() {
  bool disk_mounted = false;
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
      fstream disk(name, ios::in | ios::out | ios::binary | ios::ate);
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
          cout << "11. read disk\n";
          cout << "Enter your choice: ";
          int choice;
          cin >> choice;
          switch (choice) {
          case 1: {
            cout << "enter the name of the file: ";
            string name;
            cin >> name;
            create_file(disk, name);
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
          case 3: {
            cout << "enter the name of the file: ";
            string name;
            cin >> name;
            // check if file exists
            cout << "Coming soon!\n";
            break;
          }
          case 10: {
            cout << "disk unmounted\n";
            disk.close();
            disk_mounted = false;
            goto start;
          }
          case 11: {
            read_disk(disk);
            break;
          }
            cout << endl;
          }
        }
      }
    }
    case 3:
      return 0;
    default:
      cout << "invalid choice\n";
    }
    cout << endl;
  }
}