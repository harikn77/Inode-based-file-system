#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
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
  int inode_start;
  int data_start;
};
struct inode {
  char fname[100];
  int fsize;
  int direct[10];
  int single_indirect;
  int double_indirect;
};
struct file_info {
  int inode_num;
  int fd;
  int mode;
};

map<string, file_info> files;
map<int,string> fd_to_file;
char inode_bitmap[NUM_OF_INODES];
char data_bitmap[NUM_OF_DATA_BLOCKS];
superblock sb;
int global_fd;
void create_disk(string disk_name) {
  ofstream disk(disk_name, ios::out | ios::binary | ios::trunc);
  superblock sb;
  sb.nblocks = NUM_OF_DATA_BLOCKS;
  sb.ninodes = NUM_OF_INODES;
  sb.inode_start = 3*BLOCK_SIZE;
  sb.data_start = sb.inode_start + NUM_OF_INODE_BLOCKS * BLOCK_SIZE;
  disk.write((char *)&sb, sizeof(superblock));

  // string inode_bitmap = string(NUM_OF_INODES, '0');
  // string data_bitmap = string(NUM_OF_DATA_BLOCKS, '0');
  for (int i = 0; i < NUM_OF_INODES; i++)
    inode_bitmap[i] = '0';
  for (int i = 0; i < NUM_OF_DATA_BLOCKS; i++)
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
  // cout << "superblock: " << endl;
  // cout << "nblocks: " << sb.nblocks << endl;
  // cout << "ninodes: " << sb.ninodes << endl;
  disk.seekg(BLOCK_SIZE, ios::beg);

  disk.read(inode_bitmap, NUM_OF_INODES);
  // cout << "inode bitmap: " << endl;
  // for (int i = 0; i < NUM_OF_INODES; i++) {
  //   cout << inode_bitmap[i];
  // }
  // cout << endl;
  for (int i = 0; i < NUM_OF_INODES; i++) {
    if (inode_bitmap[i] == '1') {
      disk.seekg(3 * BLOCK_SIZE + i * INODE_SIZE, ios::beg);
      inode temp;
      disk.read((char *)&temp, sizeof(inode));
      file_info fi;
      fi.inode_num = i;
      fi.fd = -1;
      files[temp.fname] = fi;
    }
  }

  disk.seekg(2 * BLOCK_SIZE, ios::beg);
  disk.read(data_bitmap, NUM_OF_DATA_BLOCKS);

  // cout << "data bitmap: " << endl;
  // for (int i = 0; i < NUM_OF_DATA_BLOCKS; i++) {
  //   cout << data_bitmap[i];
  // }
  // cout << endl;
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
  // for (int i = 0; i < NUM_OF_DATA_BLOCKS; i++)
  //   cout << data_bitmap[i];
  // cout << endl;
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
  for (int i = 0; i < 10; i++) {
    new_inode.direct[i] = -1;
  }
  // new_inode.direct[0] = data_index;
  new_inode.single_indirect = -1;
  new_inode.double_indirect = -1;
  disk.seekp((3 * BLOCK_SIZE) + (inode_index * INODE_SIZE), ios::beg);
  disk.write((char *)&new_inode, sizeof(new_inode));
  file_info fi;
  fi.inode_num = inode_index;
  fi.fd = -1;
  files[fname] = fi;

  cout << "file created" << endl;
}
void open_file(string fname) {
  if (files.find(fname) == files.end()) {
    cout << "file does not exist" << endl;
    return;
  }
  if (files[fname].fd != -1) {
    cout << "file already opened, close it first to reopen again" << endl;
    return;
  }
  cout << "0. read mode\n";
  cout << "1. write mode\n";
  cout << "2. append mode\n";
  int mode;
  cout << "select mode: ";
  cin >> mode;
  files[fname].mode = mode;
  files[fname].fd = global_fd++;
  fd_to_file[files[fname].fd] = fname;
  string modes[3] = {"read", "write", "append"};
  cout << "file opened in " << modes[mode]
       << " mode with fd: " << files[fname].fd << endl;
}
void read_file(fstream &disk,int fd) {
  if (fd_to_file.find(fd) == fd_to_file.end()) {
    cout << "invalid fd" << endl;
    return;
  }
  string fname = fd_to_file[fd];
  if (files[fname].mode != 0) {
    cout << "file is not opened in read mode" << endl;
    return;
  }
  int inode_num = files[fname].inode_num;
  disk.seekg((3 * BLOCK_SIZE) + (inode_num * INODE_SIZE), ios::beg);
  inode temp;
  disk.read((char *)&temp, sizeof(inode));
  int fsize = temp.fsize;
  if (fsize == 0) {
    cout << "file is empty" << endl;
    return;
  }
  int direct_index = 0;
  while(direct_index < 10 && temp.direct[direct_index] != -1) {
    int data_index = temp.direct[direct_index];
    disk.seekg(sb.data_start + data_index * BLOCK_SIZE, ios::beg);
    char data[BLOCK_SIZE];
    disk.read(data, min(fsize,BLOCK_SIZE));
    cout << data;
    direct_index++;
    fsize -= BLOCK_SIZE;
  }
  cout << endl;
}
string read_from_user(){
  //read one char at a time until '$' is encountered
  cout << "Enter the content after appending '$' to the end. \n";
  string s;
  char c;
  int i = 0;
  while(cin.get(c)){
    if(i == BLOCK_SIZE-1){
      cout << "Maximum charachers allowed is " << BLOCK_SIZE << endl;
      break;
    }
    if(c == '$'){
      break;
    }
    s += c;
    i++;
  }
  return s;
}
void delete_file(fstream &disk, string fname) {
  if (files.find(fname) == files.end()) {
    cout << "file does not exist" << endl;
    return;
  }
  if(files[fname].fd != -1){
    cout << "file is open, close it first to delete" << endl;
    return;
  }
  int inode_num = files[fname].inode_num;
  disk.seekg((3 * BLOCK_SIZE) + (inode_num * INODE_SIZE), ios::beg);
  inode temp;
  disk.read((char *)&temp, sizeof(inode));
  int direct_index = 0;
  while(direct_index < 10 && temp.direct[direct_index] != -1) {
    int data_index = temp.direct[direct_index];
    data_bitmap[data_index] = '0';
    direct_index++;
  }
  inode_bitmap[inode_num] = '0';
  disk.seekp(BLOCK_SIZE, ios::beg);
  disk.write(inode_bitmap, NUM_OF_INODES);
  disk.seekp(2 * BLOCK_SIZE, ios::beg);
  disk.write(data_bitmap, NUM_OF_DATA_BLOCKS);
  files.erase(fname);
  cout << "file deleted" << endl;
}
void write_file(fstream &disk,int fd) {
  if (fd_to_file.find(fd) == fd_to_file.end()) {
    cout << "invalid fd" << endl;
    return;
  }
  string fname = fd_to_file[fd];
  if (files[fname].mode != 1) {
    cout << "file is not opened in write mode" << endl;
    return;
  }
  int inode_num = files[fname].inode_num;
  disk.seekg((3 * BLOCK_SIZE) + (inode_num * INODE_SIZE), ios::beg);
  inode temp;
  disk.read((char *)&temp, sizeof(inode));
  string userinput = read_from_user();
  int fsize = userinput.length();
  int direct_index = 0;
  while(direct_index < 10 && temp.direct[direct_index] != -1){
    
  }
}
void append_file(fstream &disk,int fd) {
  if (fd_to_file.find(fd) == fd_to_file.end()) {
    cout << "invalid fd" << endl;
    return;
  }
  string fname = fd_to_file[fd];
  if (files[fname].mode != 2) {
    cout << "file is not opened in append mode" << endl;
    return;
  }
  int inode_num = files[fname].inode_num;
  disk.seekg(sb.inode_start + (inode_num * INODE_SIZE), ios::beg);
  inode temp;
  disk.read((char *)&temp, sizeof(inode));
  int fsize = temp.fsize;
  int direct_index = 0;
  while(direct_index < 10 && temp.direct[direct_index] != -1) {
    direct_index++;
  }
  if (direct_index == 10) {
    cout << "file is full" << endl;
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
  
  if (data_index == -1) {
    cout << "no more data blocks, disk is full" << endl;
    return;
  }
  string user_data = read_from_user();
  data_bitmap[data_index] = '1';
  disk.seekp(2*BLOCK_SIZE, ios::beg);
  disk.write(data_bitmap, NUM_OF_DATA_BLOCKS);
  temp.direct[direct_index] = data_index;
  temp.fsize += user_data.length();
  disk.seekp((3 * BLOCK_SIZE) + (inode_num * INODE_SIZE), ios::beg);
  disk.write((char *)&temp, sizeof(inode)); 
  disk.seekp(sb.data_start + data_index * BLOCK_SIZE, ios::beg);
  disk.write(user_data.c_str(), user_data.length());
  cout << "file appended" << endl;
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
            open_file(name);
            break;
          }
          case 3: {
            cout << "enter the fd: ";
            int fd;
            cin >> fd;
            read_file(disk, fd);
            break;
          }
          case 5: {
            cout << "enter the fd: ";
            int fd;
            cin >> fd;
            append_file(disk, fd);
            break;
          }
          case 6: {
            cout << "enter the fd: ";
            int fd;
            cin >> fd;
            if(fd_to_file.find(fd) == fd_to_file.end()) 
              cout << "invalid fd\n";
            else {
              string fname = fd_to_file[fd];
              files[fname].fd = -1;
              fd_to_file.erase(fd);
              cout << "file is closed\n";
            }
            break;
          }
          case 7: {
            cout << "enter the name of the file: ";
            string name;
            cin >> name;
            delete_file(disk, name);
            break;
          }
          case 8: {
            cout << "files: ";
            for (auto it = files.begin(); it != files.end(); it++) {
              cout << it->first << ", ";
            }
            cout << endl;
            break;
          }
          case 9: {
            cout << "open files: ";
            for (auto it = fd_to_file.begin(); it != fd_to_file.end(); it++) {
              cout << it->second << ", ";
            }
            cout << endl;
            break;
          }
          case 10: {
            cout << "disk unmounted\n";
            files.clear();
            fd_to_file.clear();
            global_fd = 0;
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