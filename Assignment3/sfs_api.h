#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#define MAXTYPE 4
#define MAXFILENAME 20
#define MAX_BLOCKS 30000
#define DISK "acm_sfs.disk"
#define MAXFD 100
#define BLOCK_SIZE 1024
#define MAXINODES 1024
#define DATA_PTRS 128

typedef struct sBlock {
	int magic;
	int blockSize;
	int fileSystemSize;
	int rootDir;
} SuperBlock;                                                                        
SuperBlock sBlock;
typedef struct iNode {
	int size; 
	int inodePtrs[DATA_PTRS];
	void *inodePtr;
} INode;
typedef struct fdEntry {
	char name[50];
	int pp;
	int pp_block;
	int inNum; 
} Fd;
typedef struct fdt {
	int size;
	Fd slot[MAXFD];
	int nextFree;
} FdTable;
FdTable *fdTable; 
typedef struct folder {
	char name[(MAXFILENAME+MAXTYPE)+1];
	INode inode;
} DirectoryEntry;
typedef struct rd {
	int size;
	DirectoryEntry slot[MAXINODES];
	int nextFree; 
} Directory;
FdTable *fdTable; 
Directory root;
typedef struct fb {
	int freeBlocks[MAX_BLOCKS];
	int numberFree;
}FreeBlocks;
FreeBlocks freeBlocks;
int getNext;

void mksfs(int fresh);
int sfs_get_next_filename(char *fname);
int sfs_GetFileSize(const char* path);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_fread(int fileID, char *buf, int length);
int sfs_fwrite(int fileID, const char *buf, int length);
int sfs_fseek(int fileID, int loc);
int sfs_remove(char *file);
int init_fresh_disk(char *filename, int block_size, int num_blocks);
int init_disk(char *filename, int block_size, int num_blocks);
int read_blocks(int start_address, int nblocks, void *buffer);
int write_blocks(int start_address, int nblocks, void *buffer);
