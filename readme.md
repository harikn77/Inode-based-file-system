## Inode based file system Implementation
### Architecture overview 
* Disk size : 500MB
* Block size: 4000B
* Number of blocks = 125k
* Number of inode blocks = 5k
* Number of data blocks = 120k
* Inode size = 250B
* Number of inodes = 80k
	* Therefore maximum number of files supported in this disk is 80k
### Assumptions 
* Only direct block pointers are implemented; an inode has ten direct block pointers, allowing for a maximum file size of 40KB on this disc. 
* At any given time, a file can only be opened in one mode. Close the previous mode before switching to another. 
