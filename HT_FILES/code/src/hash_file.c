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

void printallrecs(Index_info* index){
	for(int j = 0; j < index->size; j++){

		void* test = BF_Block_GetData(index->hash_table[j]);
		Bucket_info* bucktest = test;

		printf("BLOCK REMAINING RECORDS:%d\n-----------------\n", 8 - bucktest->rec_num);
		for(int i = 0; i < bucktest->rec_num; i++){
			Record* rectest = (test + sizeof(Bucket_info) + i * sizeof(Record));
			printf("BLOCK:%d NAME:%s  ID:%d \n", j, rectest->name, rectest->id);
		}		
		printf("\n");
	}
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
		printf("ID1:%d\n", record.id);
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
		printf("ID2:%d\n", record.id);
		Record* rec = (data + sizeof(Bucket_info) + (buck_info->rec_num * sizeof(Record)));
		*rec = record;
		
		buck_info->rec_num ++;
		free(msb);


		if(record.id == 20){
			printf("\n\n\n\n\n\n");
			printallrecs(index);
		}
		return HT_OK;
	}

	//double array
	if(buck_info->local_depth  == index->global_depth){
		printf("double:ID:%d\n", record.id);

		Index_info* newindex;

		BF_Block_Init(&block);
  		CALL_BF(BF_AllocateBlock(index->file_desc, block));

		data = BF_Block_GetData(block);
		newindex = data;

		newindex->bucket_num = 0;
		newindex->size = index->size * 2;
		newindex->file_desc = index->file_desc;
		newindex->global_depth = index->global_depth + 1;
		newindex->bucket_num = index->bucket_num;
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

				}				

				void* insertdata = BF_Block_GetData(newindex->hash_table[rehash_key]);
				Bucket_info* buck_info = insertdata;

				Record* newrec = (insertdata + sizeof(Bucket_info) + (buck_info->rec_num * sizeof(Record)));
				*newrec = *rec;

				buck_info->rec_num ++;
				free(msb);
			}
		}

		int lastkey = final_key_index(record.id, newindex->global_depth);
		void* oldblock_data = BF_Block_GetData(newindex->hash_table[lastkey]);
		Bucket_info* oldblock_info = oldblock_data;

		//split the bucket
		if(newindex->global_depth > oldblock_info->local_depth && newindex->max_rec == oldblock_info->rec_num){
			printf("ID3:%d\n", record.id);
			BF_Block* new_block;

			BF_Block_Init(&new_block);

			BF_AllocateBlock(index->file_desc, new_block);

			void* newblock_data = BF_Block_GetData(new_block);

			Bucket_info* newbucket_info = newblock_data;
			newbucket_info->local_depth = newindex->global_depth;
			newbucket_info->rec_num = 0;
			newindex->hash_table[lastkey] = new_block;
			newindex->bucket_num += 1;
			
			//remaining recs, should be inserted back to the old bucket(not hashed to the new one)
			Record remrecs[newindex->max_rec];
			int remnum = 0;

			Record* newrec;
			//Hashes into 2 buckets only (new and the one before the split)
			for(int i = 0; i < oldblock_info->rec_num; i++){

				Record* splitrec = (oldblock_data + sizeof(Bucket_info)) + i * sizeof(Record);
				int split_hash = final_key_index(splitrec->id, newindex->global_depth);

				//Goes inside the alone bucked
				if(split_hash == lastkey){
					newrec = (newblock_data + sizeof(Bucket_info) + (newbucket_info->rec_num * sizeof(Record)));
					*newrec = *splitrec;
					newbucket_info->rec_num ++;
				}
				else{
					remrecs[remnum] = *splitrec;
					remnum ++;
				}
			}
			//Insert last record
			newrec = (newblock_data + sizeof(Bucket_info) + (newbucket_info->rec_num * sizeof(Record)));
			*newrec = record;
			newbucket_info->rec_num ++;


			//Insert the remaining records to the old bucket
			Record nullrec;
			for(int i = 0; i < oldblock_info->rec_num; i++){
				Record* oldrec = (oldblock_data + sizeof(Bucket_info) + i * sizeof(Record));

				if(i <= remnum)
					*oldrec = remrecs[i];
				else
					*oldrec = nullrec;
			}
			oldblock_info->rec_num = remnum;
			oldblock_info->local_depth += 1;

						//////destroy old hash////

			open_files[indexDesc] = newindex;

			// printallrecs(newindex);
			return HT_OK;
		}
	}



	if(index->global_depth > buck_info->local_depth && index->max_rec == buck_info->rec_num){
			printf("ID4 SPLIT:%d\n", record.id);
			BF_Block* new_block;

			BF_Block_Init(&new_block);

			BF_AllocateBlock(index->file_desc, new_block);

			void* newblock_data = BF_Block_GetData(new_block);

			Bucket_info* newbucket_info = newblock_data;
			newbucket_info->local_depth = index->global_depth;
			newbucket_info->rec_num = 0;
			index->hash_table[arraykey] = new_block;
			index->bucket_num += 1;
			
			//remaining recs, should be inserted back to the old bucket(not hashed to the new one)
			Record remrecs[index->max_rec];
			int remnum = 0;

			Record* newrec;
			//Hashes into 2 buckets only (new and the one before the split)
			for(int i = 0; i < buck_info->rec_num; i++){

				Record* splitrec = (data + sizeof(Bucket_info)) + i * sizeof(Record);
				int split_hash = final_key_index(splitrec->id, index->global_depth);

				//Goes inside the alone bucked
				if(split_hash == arraykey){
					newrec = (newblock_data + sizeof(Bucket_info) + (newbucket_info->rec_num * sizeof(Record)));
					*newrec = *splitrec;
					newbucket_info->rec_num ++;
				}
				else{
					remrecs[remnum] = *splitrec;
					remnum ++;
				}
			}
			//Insert last record
			newrec = (newblock_data + sizeof(Bucket_info) + (newbucket_info->rec_num * sizeof(Record)));
			*newrec = record;
			newbucket_info->rec_num ++;


			//Insert the remaining records to the old bucket
			Record nullrec;
			for(int i = 0; i < buck_info->rec_num; i++){
				Record* oldrec = (data + sizeof(Bucket_info) + i * sizeof(Record));

				if(i <= remnum)
					*oldrec = remrecs[i];
				else
					*oldrec = nullrec;
			}
			buck_info->rec_num = remnum;
			buck_info->local_depth += 1;

			printallrecs(index);
			return HT_OK;
		}
	
	// printf("HEHEHEHHEHE" );
	void* dat = BF_Block_GetData(index->hash_table[arraykey]);
	Bucket_info* b;
	b = dat;
	printf("%p, numrec:%d", index->hash_table[arraykey], b->rec_num);
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

