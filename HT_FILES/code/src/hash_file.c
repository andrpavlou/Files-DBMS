#include <stdlib.h>
#include <stdio.h>
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

typedef struct{
  int global_depth;
  int file_desc;
  int size;
  int bucket_num;
  int max_rec;
  int last_block_id;
  int* block_ids;
  BF_Block** hash_table;
}Index_info;

typedef struct{
  int local_depth;
  int rec_num;
  int unpin;
}Bucket_info;


Index_info* open_files[MAX_OPEN_FILES];


HT_ErrorCode HT_Init(){
  //insert code here
  
	return HT_OK;
}


//https://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
unsigned int hash_func(unsigned int key) {

  int c2=0x27d4eb2d; // a prime or an odd constant
  key = (key ^ 61) ^ (key >> 16);
  key = key + (key < 3);
  key = key ^ (key >> 4);
  key = key * c2;
  key = key ^ (key >> 15);
  return key;
}

void intToBinary(int key, char* string_key) {
    int bits = sizeof(key) * 8;
	char* result = malloc(32 * sizeof(char));
	char* temp = malloc(32 * sizeof(char));

	char cbits[2];
	
    for (int i = bits - 1; i >= 0; i--) {
        int bit = (key >> i) & 1;

		sprintf(cbits, "%d", bit);
		strcat(temp, cbits);
    }

	strcpy(string_key, temp);

	free(temp);
	free(result);
}
void most_significant_bits(char* msb, char* string_key, int gdepth){
	string_key = (string_key + strlen(string_key) - gdepth);
	strncpy(msb, string_key, gdepth);

}

int stringToInt(char* msb){
	int size = strlen(msb);
	int total = 0;

	for(int i = 0; i < strlen(msb); i++){
		int power = pow(2, size - i - 1);
		if(msb[i] == '1')
			total += power;
	}
	return total;
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
  	int power = pow(2, depth);

  	index = data;
  	index->global_depth = depth;
	index->file_desc = file_desc;
	index->bucket_num = 0;
	index->last_block_id = 0;
  	index->max_rec = (BF_BLOCK_SIZE - sizeof(Bucket_info)) / (sizeof(Record) + 1); //prepei na arxikopoihthei alliws

	index->size = power;

  	index->hash_table = malloc(index->size * sizeof(BF_Block*));
	index->block_ids = malloc(index->size * sizeof(int));

  	BF_Block_Destroy(&block);

  	// BF_Close(file_desc);

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
	info->file_desc = file_desc;

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

	// free(index->hash_table);/////// seg fault
	
	BF_CloseFile(index->file_desc);

	return HT_OK;
}

int final_key_index(int id, int bit_num){

	int key = hash_func(id);
	char str_key[32];
	intToBinary(key, str_key);
	char* msb = malloc(bit_num * sizeof(char));
	most_significant_bits(msb, str_key, bit_num);
	int arraykey = stringToInt(msb);
	free(msb);
	return arraykey;
}

void printallrecs(Index_info* index){
	
	BF_Block* block;
	BF_Block_Init(&block);

	printf("INDEX SIZE:%d GLOBAL DEPTH:%d\n-----------------\n", index->size, index->global_depth);
	for(int j = 0; j < index->size; j++){
		BF_GetBlock(index->file_desc, index->block_ids[j], block);

		void* test = BF_Block_GetData(block);
		Bucket_info* bucktest = test;
		BF_Block_SetDirty(block);
		BF_UnpinBlock(block);

		printf("REMAINING RECORDS:%d\nLOCAL DEPTH:%d\n-----------------\n", 8 - bucktest->rec_num, bucktest->local_depth);
		for(int i = 0; i < bucktest->rec_num; i++){
			Record* rectest = (test + sizeof(Bucket_info) + i * sizeof(Record));
			printf("BLOCK:%d BLOCK_ID:%d NAME:%s  ID:%d   CITY:%s\n", j, index->block_ids[j], rectest->name, rectest->id, rectest->city);
		}	
		printf("\n");
	}
		BF_Block_Destroy(&block);	
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
	
	Index_info* index = open_files[indexDesc];
	BF_Block* block;
	BF_Block_Init(&block);

	if(index->bucket_num == 0){
		for(int i = 0; i < index->size; i++){
			BF_AllocateBlock(index->file_desc, block);

			index->bucket_num ++;
			index->last_block_id ++;
			index->block_ids[i] = index->last_block_id;

			void* data = BF_Block_GetData(block);
			Bucket_info* bucket = data;

			BF_Block_SetDirty(block);
			BF_UnpinBlock(block);

			bucket->rec_num = 0;
			bucket->local_depth = index->global_depth;


		}
		BF_Block_Destroy(&block);
		int arraykey = final_key_index(record.id, index->global_depth);

		BF_Block_Init(&block);
		BF_GetBlock(index->file_desc, index->block_ids[arraykey], block);

		void* recdata = BF_Block_GetData(block);
		Bucket_info* insert_buck = recdata;
		
		Record* rec;
		rec = (recdata + sizeof(Bucket_info) + insert_buck->rec_num * sizeof(Record));
		*rec = record;

		insert_buck->rec_num ++;
		
		BF_Block_Destroy(&block);
		return HT_OK;
	}


	int array_key = final_key_index(record.id, index->global_depth);

	BF_Block_Init(&block);
	BF_GetBlock(index->file_desc, index->block_ids[array_key], block);

	void* data = BF_Block_GetData(block);
	Bucket_info* buck = data;



	
	if(buck->rec_num < index->max_rec){
		
		
		Record* rec;
		rec = ((data + sizeof(Bucket_info)) + buck->rec_num * sizeof(Record));
		*rec = record;
		buck->rec_num++;


		BF_Block_SetDirty(block);
		BF_UnpinBlock(block);
		BF_Block_Destroy(&block);

		return HT_OK;
	}

	
	

	//split buddies
	if(index->global_depth - buck->local_depth == 1 && buck->rec_num == index->max_rec){
		BF_Block* newblock;
		BF_Block_Init(&newblock);
		CALL_BF(BF_AllocateBlock(index->file_desc, newblock));
			

		void* insertdata = BF_Block_GetData(newblock);

		

		Bucket_info* newbuck = insertdata;

		index->last_block_id ++ ;
		index->block_ids[array_key] = index->last_block_id;


		buck->local_depth ++;

		newbuck->local_depth = index->global_depth;
		newbuck->rec_num = 0;

		Record rem_rec[index->max_rec];
		int rems  = 0;

		//iterate through all records of old block;
		for(int i = 0; i < buck->rec_num; i++){
			Record* rec = (data + sizeof(Bucket_info) + i * sizeof(Record));

			int rehash = final_key_index(rec->id, index->global_depth);

			//insert them to new block
			if(rehash == array_key){
				Record* newblockrec = (insertdata + sizeof(Bucket_info) + newbuck->rec_num * sizeof(Record));
				*newblockrec = *rec;
				newbuck->rec_num ++;
			}
			else{
				rem_rec[rems] = *rec;
				rems++;
			}
		}

		Record nullr;
		for(int i = 0; i < index->max_rec; i++){
			Record* oldrec = (data + sizeof(Bucket_info) + i * sizeof(Record));
			*oldrec = nullr;
		}
		buck->rec_num = 0;

		for(int i = 0; i < rems; i++){
			Record* oldrec = (data + sizeof(Bucket_info) + buck->rec_num * sizeof(Record));
			*oldrec = rem_rec[i];
			buck->rec_num ++;
		}
		// BF_Block_Destroy(&newblock);
		HT_InsertEntry(indexDesc, record);
		BF_Block_SetDirty(newblock);
		BF_UnpinBlock(newblock);
		return HT_OK;
	}
	

	


	if(index->global_depth > buck->local_depth && index->global_depth - buck->local_depth != 1 && buck->rec_num == index->max_rec){
		BF_Block* newblock;
		BF_Block_Init(&newblock);
	
		

		CALL_BF(BF_AllocateBlock(index->file_desc, newblock));


		int buddie_index = array_key - pow(2, (index->global_depth - 1));

		index->last_block_id ++;
		index->block_ids[array_key] = index->last_block_id;
		index->block_ids[buddie_index] = index->last_block_id;

		void* insertdata = BF_Block_GetData(newblock);

		
		Bucket_info* newbuck = insertdata;
		newbuck->local_depth = index->global_depth - 1;
		newbuck->rec_num = 0;

		Record rem_rec[index->max_rec];
		int rems  = 0;

		

		//iterate through all records of old block;
		for(int i = 0; i < buck->rec_num; i++){
			Record* rec = (data + sizeof(Bucket_info) + i * sizeof(Record));

			int rehash = final_key_index(rec->id, index->global_depth);

			//insert them to new block
			if(rehash == array_key || buddie_index == rehash){
				Record* newblockrec = (insertdata + sizeof(Bucket_info) + newbuck->rec_num * sizeof(Record));
				*newblockrec = *rec;
				newbuck->rec_num ++;
			}
			else{
				rem_rec[rems] = *rec;
				rems++;
			}
		}

		Record nullr;
		for(int i = 0; i < index->max_rec; i++){
			Record* oldrec = (data + sizeof(Bucket_info) + i * sizeof(Record));
			*oldrec = nullr;
		}
		buck->rec_num = 0;

		for(int i = 0; i < rems; i++){
			Record* oldrec = (data + sizeof(Bucket_info) + buck->rec_num * sizeof(Record));
			*oldrec = rem_rec[i];
			buck->rec_num ++;
		}

		

		// BF_Block_Destroy(&newblock);
		HT_InsertEntry(indexDesc, record);
		BF_Block_SetDirty(newblock);
		BF_UnpinBlock(newblock);
		

		return HT_OK;
	}


	

	//Double the array
	if(buck->rec_num == index->max_rec && buck->local_depth == index->global_depth){
		// printf("\ndouble, %d", record.id);
		int newblock[index->size];

		for(int i = 0; i < index->size; i++){
			newblock[i] = index->block_ids[i];
		}

		index->size *= 2;
		free(index->block_ids);
		index->block_ids = malloc(index->size * sizeof(int*));

		for(int i = 0; i < index->size / 2; i++){
			index->block_ids[i] = newblock[i];
		}

		
		index->global_depth += 1;

		for(int i = index->size / 2; i < index->size; i++){
			index->block_ids[i] = index->block_ids[i - index->size/2];
		}
		HT_InsertEntry(indexDesc, record);
		return HT_OK;
	}


	printf("------ERROROROROROROROROOROR");
	return HT_OK;



}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {

	Index_info* index = open_files[indexDesc]; 	
	//block 13;
	printallrecs(index);
	
	
  	return HT_OK;
}