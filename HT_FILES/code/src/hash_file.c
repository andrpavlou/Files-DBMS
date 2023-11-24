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
  BF_Block** hash_table;
}Index_info;

typedef struct{
  int local_depth;
  int rec_num;
}Bucket_info;


Index_info* open_files[MAX_OPEN_FILES];


HT_ErrorCode HT_Init(){
  //insert code here
  
	return HT_OK;
}


//https://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
unsigned int hash_func(unsigned int key) {
    key = ~key + (key << 15); // key = (key << 15) - key - 1;
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057; // key = (key + (key << 3)) + (key << 11);
    key = key ^ (key >> 16);
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
  	index->max_rec = (BF_BLOCK_SIZE - sizeof(Bucket_info)) / (sizeof(Record) + 1); //prepei na arxikopoihthei alliws
	// printf("\n\n\nidndex->max_rec%d ", index->max_rec);

	index->size = power;

  

  	index->hash_table = malloc(index->size * sizeof(BF_Block*));
  

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

	return arraykey;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here
  	Index_info* index;
 	BF_Block* block;

  	void* data;
  	index = open_files[indexDesc];
  	int ht_size = index->size;
  	int num_buck = index->bucket_num;
  	int filedesc = index->file_desc;

	int gdepth = index->global_depth;
	int key = hash_func(record.id);

	char str_key[32];
	intToBinary(key, str_key);
	char* msb = malloc(gdepth * sizeof(char));
	most_significant_bits(msb, str_key, gdepth);
	int arraykey = stringToInt(msb);




  	BF_Block** hash_table = index->hash_table;
	Bucket_info* buck_info;  

	//no buckets available
	if(num_buck == 0){
    	/*Allocate initial buckets and their data if first insert*/
		for(int i = 0; i < ht_size; i ++){
			BF_Block_Init(&hash_table[i]);
			CALL_BF(BF_AllocateBlock(filedesc, hash_table[i]));


			data = BF_Block_GetData(hash_table[i]);

			buck_info = data;

			buck_info->local_depth = index->global_depth;
			buck_info->rec_num = 0;

			index->bucket_num ++;
			data = BF_Block_GetData(hash_table[i]);
    	}
		data = BF_Block_GetData(hash_table[arraykey]);

		Record* rec = (data + sizeof(Bucket_info) + (buck_info->rec_num * sizeof(Record)));	
    	*rec = record;

		buck_info->rec_num ++;

		free(msb);
		return HT_OK;
	}
	data = BF_Block_GetData(hash_table[arraykey]);
	buck_info = data;

	if(buck_info->rec_num < index->max_rec){
		Record* rec = (data + sizeof(Bucket_info) + (buck_info->rec_num * sizeof(Record)));
		*rec = record;
		
		buck_info->rec_num ++;
		free(msb);

		return HT_OK;
	}
	//double array


	if(buck_info->local_depth  == index->global_depth){


		Index_info* newindex;

		BF_Block_Init(&block);
  		CALL_BF(BF_AllocateBlock(index->file_desc, block));

		data = BF_Block_GetData(block);
		newindex = data;

		newindex->bucket_num = 0;
		newindex->size = index->size * 2;
		newindex->file_desc = index->file_desc;
		newindex->global_depth = index->global_depth + 1;
		newindex->max_rec = (BF_BLOCK_SIZE - sizeof(Bucket_info)) / (sizeof(Record) + 1); 
		newindex->hash_table = malloc(newindex->size * sizeof(BF_Block*));
		
		//allocate everything null
		for(int i = 0; i < newindex->size; i++)
			newindex->hash_table[i] = NULL;



		for(int i = 0; i < index->size; i ++){
				data = BF_Block_GetData(index->hash_table[i]);
				buck_info = data;

			for (int j = 0; j < buck_info->rec_num ; j++){
				Record* rec = (data + sizeof(Bucket_info) + sizeof(Record) * j);

				int id = rec->id;
				int rehash_key = final_key_index(id, newindex->global_depth);
				// printf("rk: %d \n\n\n", rec->id);

				if(rehash_key % 2  == 0 && newindex->hash_table[rehash_key] == NULL && newindex->hash_table[rehash_key + 1] == NULL){
					BF_Block_Init(&newindex->hash_table[rehash_key]);
					BF_Block_Init(&newindex->hash_table[rehash_key + 1]);

					CALL_BF(BF_AllocateBlock(newindex->file_desc, newindex->hash_table[rehash_key]));
					void* newdata = BF_Block_GetData(newindex->hash_table[rehash_key]);

					Bucket_info* newbucket_info;
					newbucket_info = newdata;
					
					newbucket_info->rec_num = 0;
					newbucket_info->local_depth = newindex->global_depth - 1;

					newindex->hash_table[rehash_key + 1] =  newindex->hash_table[rehash_key];

					// printf("here\n");
				}
				if(rehash_key % 2 - 1 == 0 && newindex->hash_table[rehash_key] == NULL && newindex->hash_table[rehash_key - 1] == NULL){
					BF_Block_Init(&newindex->hash_table[rehash_key]);
					BF_Block_Init(&newindex->hash_table[rehash_key - 1]);

					CALL_BF(BF_AllocateBlock(newindex->file_desc, newindex->hash_table[rehash_key]));
					void* newdata = BF_Block_GetData(newindex->hash_table[rehash_key]);

					Bucket_info* newbucket_info;
					newbucket_info = newdata;
					
					newbucket_info->rec_num = 0;
					newbucket_info->local_depth = newindex->global_depth - 1;


					newindex->hash_table[rehash_key - 1] =  newindex->hash_table[rehash_key];

					// printf("\n %p \n %p ", newindex->hash_table[rehash_key - 1], newindex->hash_table[rehash_key]);

				}				
				/////////////////////////////////////////

				// printf(" %d ", rehash_key);
				void* insertdata = BF_Block_GetData(newindex->hash_table[rehash_key]);
				Bucket_info* buck_info = insertdata;

				Record* newrec = (insertdata + sizeof(Bucket_info) + (buck_info->rec_num * sizeof(Record)));
				*newrec = *rec;

				buck_info->rec_num ++;
				// printf(" %d ", buck_info->rec_num);
				free(msb);
			}
		}



		int lastkey = final_key_index(record.id, newindex->global_depth);
		/////////split bucket
		BF_Block* block;
		BF_Block_Init(&block);
		block = newindex->hash_table[lastkey];
		void* insertlast = BF_Block_GetData(newindex->hash_table[lastkey]);

		printf(" not destroyed::%p ", newindex->hash_table[0]);
		printf("\n not  destroyed::%p ", newindex->hash_table[1]);
		BF_Block_Destroy(&newindex->hash_table[0]);
		printf("\n destroyed::%p ", newindex->hash_table[0]);
		printf("\n destroyed::%p ", newindex->hash_table[1]);
		
		// void* insertlast = BF_Block_GetData(newindex->hash_table[rehash_key]);
		// Bucket_info* buck_info = insertlast;


		// 	testtestdata = BF_Block_GetData(newindex->hash_table[3]);
		// 	buck_info = testtestdata;
		// 	printf(" %d ", buck_info->rec_num);


	}






	return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {

	Index_info* index = open_files[indexDesc]; 	
	// printf("printf%d ", indexDesc);
	
	void* data = BF_Block_GetData(index->hash_table[0]);
	Record* rec = (data + sizeof(Bucket_info) + (8 * sizeof(Record)));


	// printf("%p ", index->hash_table[0]);


	///otan ginei close;
	// BF_Block_SetDirty(index->hash_table[0]);
	// CALL_BF(BF_UnpinBlock(index->hash_table[0]));	
	
  	return HT_OK;
}

