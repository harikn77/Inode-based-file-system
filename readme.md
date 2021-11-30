## Inode based file system Implementation
### Architecture overview 
* Disk size : 256KB
* Block size: 4096B
* Number of blocks = 64
* Number of inode blocks = 5
* 1 superblock,2 bitmap blocks
* Number of data blocks = 56
* Inode size = 256B
* Number of inodes = 80
	* Therefore maximum number of files supported in this disk is 80
### Assumptions 
* Only direct block pointers are implemented; an inode has ten direct block pointers, allowing for a maximum file size of 40KB on this disc. 
* At any given time, a file can only be opened in one mode. Close the previous mode before switching to another. 
