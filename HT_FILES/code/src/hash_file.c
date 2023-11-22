#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20
#define HT_ERROR -1



#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    return HT_ERROR;        \
  }                         \
}


typedef struct {
	int file_desc;
	int global_depth;
  	BF_Block** hash_table;
}Index_info;

Index_info* open_files[MAX_OPEN_FILES];


HT_ErrorCode HT_Init(){
  //insert code here
  
	return HT_OK;
}


//https://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
unsigned int hash32shift(unsigned int key) {
    key = ~key + (key << 15); // key = (key << 15) - key - 1;
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057; // key = (key + (key << 3)) + (key << 11);
    key = key ^ (key >> 16);
    return key;
}

void intToBinary(int num, char* string_key) {
    int bits = sizeof(num) * 8;
	char* result = malloc(32 * sizeof(char));
	char* temp = malloc(32 * sizeof(char));

	char cbits[2];
	
    for (int i = bits - 1; i >= 0; i--) {
        int bit = (num >> i) & 1;

		sprintf(cbits, "%d", bit);
		strcat(temp, cbits);
    }

	strcpy(string_key, temp);

	free(temp);
	free(result);
}
void most_significant_bits(char* string, char* string_key, int global_depth){
	strncpy(string, string_key, global_depth);
}


HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
	int file_desc;
  	BF_Block* block;
  	Index_info* index;



	CALL_BF(BF_CreateFile(filename)); 
	CALL_BF(BF_OpenFile(filename, &file_desc));

  	BF_Block_Init(&block);
  	CALL_BF(BF_AllocateBlock(file_desc, block));

  	void* data;
	data = BF_Block_GetData(block);

  	index = data;
  	index->global_depth = depth;
	index->file_desc = file_desc;
  
  	int power = pow(2, depth);

  	index->hash_table = malloc(power * sizeof(BF_Block*));
  

  	BF_Block_Destroy(&block);
  	BF_Close(file_desc);

	return HT_OK;
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
    BF_Block* block;
    BF_Block_Init(&block);


    int file_desc;
    CALL_BF(BF_OpenFile(fileName, &file_desc));
    
    CALL_BF(BF_GetBlock(file_desc, 0, block));
    void* data;

    data = BF_Block_GetData(block);
    Index_info* info;
    info = data;

	for(int i = 0; i < MAX_OPEN_FILES; i++){
		if(open_files[i] == NULL){
			open_files[i] = info;
			*indexDesc = i;
			BF_Block_Destroy(&block);
			return HT_OK;
		}
	}
	BF_Block_Destroy(&block);
    return HT_ERROR;
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
	//insert code here
	///////////////////////////// set_dirty!?!?!??!?!?!?!

	Index_info* index;
	index = open_files[indexDesc];


	open_files[indexDesc] = NULL;

	free(index->hash_table);
	
	BF_CloseFile(index->file_desc);

	return HT_OK;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here
  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}

