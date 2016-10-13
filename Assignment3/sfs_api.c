#include "sfs_api.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

void mksfs(int fresh) {
	if (fresh == 1) {
		printf("Creating fresh disk\n");
		init_fresh_disk(DISK, BLOCK_SIZE, MAX_BLOCKS);
		//initialize everything to zero
		bzero(&sBlock, sizeof(SuperBlock));
		bzero(&root, sizeof(Directory));
		bzero(&freeBlocks, sizeof(FreeBlocks));
		//fdtable goes in memory
		fdTable = (FdTable*)calloc(1,sizeof(FdTable));
		write_blocks(0,1,&sBlock);
		write_blocks(1,1, &root);
		freeBlocks.numberFree = MAX_BLOCKS;
		freeBlocks.freeBlocks[0] = 1; //super block's block
		freeBlocks.freeBlocks[1] = 1; //root's block
		freeBlocks.freeBlocks[MAX_BLOCKS-1] = 1; // number of free blocks' block
		freeBlocks.numberFree = freeBlocks.numberFree-3;
		write_blocks(MAX_BLOCKS - 1, 1, &freeBlocks);
		printf("Successfully set up super block, root directory, and free block structure.\n");
	}
	else {
		printf("Loading existing disk\n");
		init_disk(DISK, BLOCK_SIZE, MAX_BLOCKS);
	}
	getNext = 0;
	(*fdTable).nextFree = 0;
	(*fdTable).size = 0;
	root.nextFree = 2;
	root.size = 3;
}

int sfs_get_next_filename(char *fname) {
	if (getNext >= root.size){getNext = 0; return getNext;}
	strcpy(fname,root.slot[getNext].name);
	getNext++;
	return getNext;
}

int sfs_GetFileSize(const char* path) {
	int i;
	for (i = 0;i<root.size;i++){
		if(strcmp(root.slot[i].name, path) == 0){return root.slot[i].inode.size;}
	}
	printf("sfs_getfilesize(%s) could not find the file %s\n", path, path);
	return -1;
}

int sfs_fopen(char *name) {
	if ((*fdTable).size >= MAXFD) {
		printf("sfs_fopen cannot open more files, we have hit the max.\n");
		return -1;
	}
	else if (strlen(name) > MAXFILENAME + MAXTYPE) {
		printf("The file name provided too long\n");
		return -1;
	}
	//this part is to open a file thats already on the disk
	int i;
	for (i = 0;i<=MAXINODES;i++){
		if(!strcmp(root.slot[i].name, name)){
			//Find the inode of the file
			int j;
			for (j = 0;j<MAXFD;j++){ //now we check if it's already open
				if (!strcmp((*fdTable).slot[j].name, name)){
					printf("%s is already open!\n", name);
					return -1;
				}
			}
			printf("Opening file: %s\n", name);
			int rVal = (*fdTable).nextFree; 
			(*fdTable).slot[rVal].inNum  = i;
			strcpy((*fdTable).slot[rVal].name,name);
			(*fdTable).slot[(*fdTable).nextFree].pp = root.slot[i].inode.size; 
			//Now that we've set up and opened the file we need to move our index to the next open spot on the fd table
			int k;
			for (k=0;j<MAXFD;k++){
				if (strlen((*fdTable).slot[k].name) ==  0){
					(*fdTable).nextFree = k;
					(*fdTable).size++;
					return rVal;
				}
			}
			//This is the case where we are able to put in a file but have no more space after that file
			(*fdTable).nextFree = -1;
			(*fdTable).size++;
			return rVal;
        }
	}
	//this part is to open a new file if it doesn't exist on disk
    if (root.size >= MAXINODES) {
        printf("No more INodes can be created\n");
		return -1;
    }
    //now we need to create the INode
	root.size++;
	int rVal = (*fdTable).nextFree;
	INode *inode = (INode*)calloc(sizeof(INode), 1);
	(*inode).size = 0;
	root.slot[root.nextFree].inode = *inode;
	memcpy(root.slot[root.nextFree].name,name,strlen(name));
	root.slot[root.nextFree].name[strlen(name)] = '\0';
    memcpy((*fdTable).slot[rVal].name, name,strlen(name));
	(*fdTable).slot[rVal].name[strlen(name)] = '\0';
    (*fdTable).slot[rVal].inNum = root.nextFree;
    //done creating inode and fixing indexes
	printf("%s has been created. Opened at fdtable index %d\n", name, rVal);
	int l;
	for (l=0;l<MAXINODES;l++){
        	if (strlen(root.slot[l].name)<=0){
                root.nextFree = l;
				break;
            }
    }
    write_blocks(1,1,&root);
    //same as above when we opened an existing file for moving the fd table's index
    for (l=0;l<MAXFD;l++){
        if (strlen((*fdTable).slot[l].name) ==  0){
        	(*fdTable).nextFree = l;
			(*fdTable).size++;
        	return rVal;
        	}
        }	
	(*fdTable).size ++;
	(*fdTable).nextFree = -1;
	return rVal;	
}

int sfs_fclose(int fileID){
	//Check if file is open
	if (strlen((*fdTable).slot[fileID].name)== 0) {
		printf("The fdtable doesn't have a file open at %d\n", fileID);
		return -1;
	}
	//take out of fdtable
	strcpy((*fdTable).slot[fileID].name, "");
	(*fdTable).slot[fileID].inNum = -1;
	(*fdTable).size--;
	//same as before, reset the fdtable index
	int l;
    for (l=0;l<MAXFD;l++){
        if (strlen((*fdTable).slot[l].name) ==  0){
            (*fdTable).nextFree = l;
        	return 0;
		}
    }
	printf("%s was closed.\n", (*fdTable).slot[fileID].name);
	return 0;
}

int sfs_fseek(int fileID, int loc){
	int location,block_location;
    if (strlen((*fdTable).slot[fileID].name) == 0){
		printf("Could not find a file at index %d in the fdtable\n", fileID);
        return -1;
    }
	else {
		location = loc % BLOCK_SIZE; 
		block_location = loc/BLOCK_SIZE;
		(*fdTable).slot[fileID].pp = location;
		(*fdTable).slot[fileID].pp_block = block_location;
		return 0;
	}
}

int sfs_remove(char *file) {
	int i,j,k;
	for (i=0;i<=MAXINODES;i++){
        if(!strcmp(root.slot[i].name, file)){
            for (k = 0;k<MAXFD;k++){
                if (!strcmp((*fdTable).slot[k].name, file)) {
                    printf("File closed.\n"); 
					sfs_fclose(k);
					break;
                }
            }
			for (j = 0;j<DATA_PTRS;j++){
				if (root.slot[i].inode.inodePtrs[j] == 0) break;
				freeBlocks.freeBlocks[root.slot[i].inode.inodePtrs[j]] = 0;
				freeBlocks.numberFree++;
				root.slot[i].inode.inodePtrs[j] = 0;
			}		
			root.slot[i].inode.size = 0;
			root.slot[i].name[0] = '\0';
			write_blocks(1,1,&root);
			write_blocks(MAX_BLOCKS-1, 1, &freeBlocks);
			return 0;
		}
	}
	printf("Couldn't find the file to be removed\n");
	return -1; 
}

int sfs_fread(int fileID, char *buf, int length){
	int dataptrs[DATA_PTRS];
	char *temp = calloc(BLOCK_SIZE,1);
	if (strlen((*fdTable).slot[fileID].name) == 0){
        printf("The fdtable doesn't have a file open at %d\n", fileID);
        return -1;
    }
	int pp = (*fdTable).slot[fileID].pp;
	int ppblock = (*fdTable).slot[fileID].pp_block; 
    int unread = length;
	if (length > root.slot[(*fdTable).slot[fileID].inNum].inode.size) {
		printf("%d bytes can't be read from %d: because %s only has %d bytes. %d bytes were copied.\n", length, fileID,(*fdTable).slot[fileID].name, root.slot[(*fdTable).slot[fileID].inNum].inode.size,root.slot[(*fdTable).slot[fileID].inNum].inode.size);
		unread = root.slot[(*fdTable).slot[fileID].inNum].inode.size;
	}
	int sizeRead = 0;
	printf("Reading %d bytes from %d in fdtable.\n", unread, fileID);
	memcpy(dataptrs,root.slot[(*fdTable).slot[fileID].inNum].inode.inodePtrs,sizeof(int)*DATA_PTRS);
	while (unread > 0){
		temp=calloc(1,BLOCK_SIZE);	
		if (pp >= BLOCK_SIZE) {
			ppblock += 1;
			pp = 0;
		}
		read_blocks(root.slot[(*fdTable).slot[fileID].inNum].inode.inodePtrs[ppblock],1,temp);
		temp += pp;
		if (unread > BLOCK_SIZE - pp){
			memcpy(buf+sizeRead,temp,BLOCK_SIZE-pp);
			unread -= (BLOCK_SIZE - pp);
			sizeRead += (BLOCK_SIZE - pp);
			pp+=BLOCK_SIZE-pp;
		}
		else {
			memcpy(buf+sizeRead,temp,unread);
			sizeRead += unread;
			pp += unread;
			unread -= unread;
		}
	}	
	(*fdTable).slot[fileID].pp_block = ppblock;
	sfs_fseek(fileID, pp);	
	return sizeRead;
}

int sfs_fwrite(int fileID, const char *buf, int length){
	if (strlen((*fdTable).slot[fileID].name) == 0){
		printf("Could not find a file at index %d in the fdtable\n", fileID);
		return length;
	} 
	printf("Writing %d bytes to file at fdtable index %d\n", length, fileID);
	void* temp = calloc(BLOCK_SIZE, 1);
	int used = 0;
	int bytesWritten = 0;	
	int bytesLeft = length;
	int spaceLeft = BLOCK_SIZE - (*fdTable).slot[fileID].pp;
	int blocknumber = (length/BLOCK_SIZE + (length%BLOCK_SIZE != 0));
	if (length%BLOCK_SIZE > spaceLeft) {blocknumber++;}
	int i;
	for (i = 0;i<DATA_PTRS;i++){
		if (root.slot[(*fdTable).slot[fileID].inNum].inode.inodePtrs[i]!= 0){ used++;} 
		else break;
	}
	int additionalPointersNeeded = (*fdTable).slot[fileID].pp_block + blocknumber - used;	
	if (freeBlocks.numberFree < additionalPointersNeeded){
		printf("sfs_fwrite error: %d blocks cannot be written with %d pointers to blocks.\n", additionalPointersNeeded, freeBlocks.numberFree);
		return -1;
	}
	if (DATA_PTRS - used - additionalPointersNeeded < 0){
		INode *nInode = (INode*)calloc(1,sizeof(INode));
		(*nInode).size = 0;
		root.slot[(*fdTable).slot[fileID].inNum].inode.inodePtr = nInode;
		return -1;
	}
	int j,blockToWriteTo,numBytesToWrite;
	for (i=1;i<= blocknumber;i++){
		if ((*fdTable).slot[fileID].pp == BLOCK_SIZE || (*fdTable).slot[fileID].pp == 0){
			for (j=0;j<MAX_BLOCKS;j++){
				if (freeBlocks.freeBlocks[j] == 0){
					blockToWriteTo = j;
					root.slot[(*fdTable).slot[fileID].inNum].inode.inodePtrs[(*fdTable).slot[fileID].pp_block] = blockToWriteTo;
					(*fdTable).slot[fileID].pp = 0;
					break;
				}
			}
		}
		else blockToWriteTo =  root.slot[(*fdTable).slot[fileID].inNum].inode.inodePtrs[(*fdTable).slot[fileID].pp_block];
		read_blocks(blockToWriteTo,1,temp); 
		if(bytesLeft > (BLOCK_SIZE - (*fdTable).slot[fileID].pp)){
			numBytesToWrite = BLOCK_SIZE - (*fdTable).slot[fileID].pp;
			 (*fdTable).slot[fileID].pp_block += 1;
		}
		else numBytesToWrite = bytesLeft;
		bytesLeft -= numBytesToWrite;
		memcpy(temp + (*fdTable).slot[fileID].pp,buf,numBytesToWrite);
		write_blocks(blockToWriteTo,1,temp);
        (*fdTable).slot[fileID].pp += numBytesToWrite;
		(*fdTable).slot[fileID].name[20] = '\0';
		bytesWritten += numBytesToWrite;
		freeBlocks.freeBlocks[blockToWriteTo] = 1;
		freeBlocks.numberFree--;
	}
	write_blocks(1,1,&root);
	write_blocks(MAX_BLOCKS-1,1,&freeBlocks);	
	printf("%d bytes written.\n", bytesWritten);
	root.slot[(*fdTable).slot[fileID].inNum].inode.size += bytesWritten;
	return bytesWritten;
}