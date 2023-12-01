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
}Index_info;

typedef struct{
  int local_depth;
  int rec_num;
}Bucket_info;

//Stores all the indexes of the files that have been opened
Index_info* open_files[MAX_OPEN_FILES];

HT_ErrorCode HT_Init(){
  
	return HT_OK;
}

//https://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
unsigned int hash_func(unsigned int	key) {
  key = (~key) + (key << 18); // key = (key << 18) - key - 1;
  key = key ^ (key >> 31);
  key = key * 21; // key = (key + (key << 2)) + (key << 4);
  key = key ^ (key >> 11);
  key = key + (key << 6);
  key = key ^ (key >> 22);
  return (int) key;
}

//Transforms an integer to binary (string) 
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
//Returns the (gdepth) lsb in string_key variable
void least_significant_bits(char* msb, char* string_key, int gdepth){
	string_key = (string_key + strlen(string_key) - gdepth);
	strncpy(msb, string_key, gdepth + 1);
}

//Transforms string to integer
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
//Calls all the needed fucntions to return the final position of the hashed key
int final_key_index(int id, int bit_num){
	//Main function that returns the key
	int key = hash_func(id);
	char str_key[32];
	//Transform the key to binary
	intToBinary(key, str_key);
	char* lsb = malloc(bit_num * sizeof(char));
	//Take the (global depth) least significant bits
	least_significant_bits(lsb, str_key, bit_num);
	int arraykey = stringToInt(lsb);

	free(lsb);
	return arraykey;
}

HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
	int file_desc;
  	BF_Block* block;
  	Index_info* index;

	CALL_BF(BF_CreateFile(filename)); 
	CALL_BF(BF_OpenFile(filename, &file_desc));


	//Allocate the first block of the file which will store the metadata of the file
  	BF_Block_Init(&block);
  	CALL_BF(BF_AllocateBlock(file_desc, block));

  	void* data;
	data = BF_Block_GetData(block);

  	index = data;
	
	index->bucket_num = 0;
	index->last_block_id = 0;
	index->size = pow(2, depth);
  	index->global_depth = depth;
	index->file_desc = file_desc;
  	index->max_rec = (BF_BLOCK_SIZE - sizeof(Bucket_info)) / (sizeof(Record) + 1);

	//Block_ids corresponds to the hash table, it contains the block ids.
	index->block_ids = malloc(index->size * sizeof(int));

	BF_Block_SetDirty(block);
	BF_UnpinBlock(block);
  	BF_Block_Destroy(&block);

	return HT_OK;
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
    BF_Block* block;
    BF_Block_Init(&block);

    int file_desc;
	//Block with id =  0, has metadata of the file
    CALL_BF(BF_OpenFile(fileName, &file_desc));
    CALL_BF(BF_GetBlock(file_desc, 0, block));
	
    void* header = BF_Block_GetData(block);
    Index_info* info;
    info = header;

	BF_Block_Destroy(&block);

	info->file_desc = file_desc;

	//Stores the metadata of the file in the first null position
	for(int i = 0; i < MAX_OPEN_FILES; i++){
		if(open_files[i] == NULL){
			open_files[i] = info;
			*indexDesc = i;
			return HT_OK;
		}
	}
    return HT_ERROR;
}


HT_ErrorCode HT_CloseFile(int indexDesc) {
	Index_info* index;
	BF_Block* block;
	BF_Block_Init(&block);

	index = open_files[indexDesc];

	//SetDiry/Unpin each block of the file
	for(int i = 0; i < index->size; i++){
		BF_GetBlock(index->file_desc, index->block_ids[i], block);
		BF_Block_SetDirty(block);
		BF_UnpinBlock(block);
	}

	free(index->block_ids);
	BF_Block_Destroy(&block);
	
	BF_CloseFile(index->file_desc);

	return HT_OK;
}



void printallrecs(Index_info* index){

	BF_Block* block;
	BF_Block_Init(&block);
	printf("INDEX SIZE:%d GLOBAL DEPTH:%d\n-----------------\n", index->size, index->global_depth);
	//Iterates through each bucket (block id = 0, contains metadata)
	for(int j = 1; j <= index->bucket_num; j++){
		(BF_GetBlock(index->file_desc, j, block));

		void* test = BF_Block_GetData(block);
		Bucket_info* bucktest = test;

		BF_Block_SetDirty(block);
		BF_UnpinBlock(block);

		printf("REMAINING RECORDS:%d\nLOCAL DEPTH:%d\n-----------------\n", 8 - bucktest->rec_num, bucktest->local_depth);
		//Iterates through, the record of each bucket and prints the information need.
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

	//No buckets have been allocated, therefore the record can not be stored
	if(index->bucket_num == 0){
		for(int i = 0; i < index->size; i++){
			//Allocate as many buckets as the starting size of the index->size = size of the index->block_ids (hash table)
			BF_AllocateBlock(index->file_desc, block);
			index->bucket_num ++;

			//In each position of block_ids store the the id of the last block allocated
			index->last_block_id ++;
			index->block_ids[i] = index->last_block_id;

			//Essential bucket data stored at the start of each bucket.
			void* data = BF_Block_GetData(block);
			Bucket_info* bucket = data;

			BF_Block_SetDirty(block);
			BF_UnpinBlock(block);

			bucket->rec_num = 0;
			bucket->local_depth = index->global_depth;
		}
		//Hash table position in which the record will be inserted, depending on the id of the record
		int arraykey = final_key_index(record.id, index->global_depth);

		BF_GetBlock(index->file_desc, index->block_ids[arraykey], block);

		void* recdata = BF_Block_GetData(block);
		Bucket_info* insert_buck = recdata;
		
		//Store the record after the bucket data, right after the last record inserted. (0 records in this case) 
		Record* rec;
		rec = (recdata + sizeof(Bucket_info) + insert_buck->rec_num * sizeof(Record));
		*rec = record;

		insert_buck->rec_num ++;
		
		BF_Block_Destroy(&block);
		return HT_OK;
	}

	int array_key = final_key_index(record.id, index->global_depth);

	BF_GetBlock(index->file_desc, index->block_ids[array_key], block);

	void* data = BF_Block_GetData(block);
	Bucket_info* buck = data;
	
	//The bucket in which record.id hashes, has space to be inserted
	if(buck->rec_num < index->max_rec){

		//Insert the record reight after the last record inserted
		Record* rec;
		rec = ((data + sizeof(Bucket_info)) + buck->rec_num * sizeof(Record));
		*rec = record;
		buck->rec_num++;
	
		BF_Block_SetDirty(block);
		BF_UnpinBlock(block);
		BF_Block_Destroy(&block);

		return HT_OK;
	}

	//Important when inserting many records, otherwise overflow will occur
	BF_Block_SetDirty(block);
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);
	

	//Case 1: Bucket is full, but local depth = global depth - 1 => only the MSB is different(Buddies) so split the buddies
	if(index->global_depth - buck->local_depth == 1 && buck->rec_num == index->max_rec){
		//Allocate a new block in which the record will be stored at the end after the split and rehash of the records of the full old block
		BF_Block* newblock;
		BF_Block_Init(&newblock);
		CALL_BF(BF_AllocateBlock(index->file_desc, newblock));
		index->bucket_num ++;

		void* insertdata = BF_Block_GetData(newblock);

		Bucket_info* newbuck = insertdata;

		index->last_block_id ++ ;
		index->block_ids[array_key] = index->last_block_id;


		buck->local_depth ++;

		newbuck->local_depth = index->global_depth;
		newbuck->rec_num = 0;

		Record rem_rec[index->max_rec]; //Stores the records that need to stay in the old block after rehashing all records
		int rems  = 0;

		//iIterate through all records of old block and rehash them.
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

		//Remove all the records of old block.
		Record nullr;
		for(int i = 0; i < index->max_rec; i++){
			Record* oldrec = (data + sizeof(Bucket_info) + i * sizeof(Record));
			*oldrec = nullr;
		}
		buck->rec_num = 0;

		//Insert to the old block the rehashed records
		for(int i = 0; i < rems; i++){
			Record* oldrec = (data + sizeof(Bucket_info) + buck->rec_num * sizeof(Record));
			*oldrec = rem_rec[i];
			buck->rec_num ++;
		}

		//Recursively insert the record now that there is space (unless we have been unlucky and need to double HT :) )
		HT_InsertEntry(indexDesc, record);

		BF_Block_SetDirty(newblock);
		BF_UnpinBlock(newblock);
		BF_Block_Destroy(&newblock);

		return HT_OK;
	}

	//Case 2: Split the bucket, but both array_key and its buddy need to point to the same new bucket
	if(index->global_depth > buck->local_depth && index->global_depth - buck->local_depth != 1 && buck->rec_num == index->max_rec){
		BF_Block* newblock;
		BF_Block_Init(&newblock);
	

		CALL_BF(BF_AllocateBlock(index->file_desc, newblock));
		index->bucket_num ++;

		//Find the index of the buddy, they differ in the MSB 
		int buddie_index = 0;
		buddie_index = array_key - pow(2, (index->global_depth - 1));

		index->last_block_id ++;
		index->block_ids[array_key] = index->last_block_id;

		//If buddie_index is negaitve it means array_key is the smaller buddy
		if(buddie_index < 0)
			buddie_index = array_key + pow(2, (index->global_depth - 1));


		index->block_ids[buddie_index] = index->last_block_id;

		void* insertdata = BF_Block_GetData(newblock);
		
		Bucket_info* newbuck = insertdata;
		newbuck->local_depth = index->global_depth - 1;
		newbuck->rec_num = 0;

		Record rem_rec[index->max_rec];
		int rems  = 0;

		//Same process as case 1
		for(int i = 0; i < buck->rec_num; i++){
			Record* rec = (data + sizeof(Bucket_info) + i * sizeof(Record));

			int rehash = final_key_index(rec->id, index->global_depth);

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

		HT_InsertEntry(indexDesc, record);

		BF_Block_SetDirty(newblock);
		BF_UnpinBlock(newblock);
		BF_Block_Destroy(&newblock);

		return HT_OK;
	}
	
	//Case 3: local depth = global depth => double the HT
	if(buck->rec_num == index->max_rec && buck->local_depth == index->global_depth){
		//Store the old values of HT
		int newblock[index->size];
		for(int i = 0; i < index->size; i++)
			newblock[i] = index->block_ids[i];


		index->global_depth += 1;

		//Double the hash table
		index->size *= 2;
		free(index->block_ids);
		index->block_ids = malloc(index->size * sizeof(int*));


		//First half of the new HT is the same as the whole old HT
		for(int i = 0; i < index->size / 2; i++)
			index->block_ids[i] = newblock[i];
		
		//Set the values of the second half of the array
		for(int i = index->size / 2; i < index->size; i++)
			index->block_ids[i] = index->block_ids[i - index->size/2];


		/* Example of double using LSB. (global depth - 1) LSB point to the same bucket
							Old HT: 		New HT:

											00 --> 1
							0 --> 1			01 --> 2
							1 --> 2			10 --> 1
											11 --> 2
		*/		
		
		//Insert the last record recursively 
		HT_InsertEntry(indexDesc, record);
		return HT_OK;
	}

	return HT_ERROR;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
    Index_info* index = open_files[indexDesc]; 
    BF_Block* block;

    Bucket_info* bucket_info;
    Record* rec;
    void* data;
    int error = -1;

    if(id == NULL){
		//Prints all records in the file
        printallrecs(index);
        return HT_OK;
    }


    BF_Block_Init(&block);
    int hash_key = final_key_index(*id ,index->global_depth);	//Hash value of the record, needed to be found
    int block_id = index->block_ids[hash_key];

	//Find the bucket info that has the record id that is needed to be found
    CALL_BF(BF_GetBlock(index->file_desc, block_id, block));
    data = BF_Block_GetData(block);
    bucket_info = data;

	BF_Block_SetDirty(block);
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);

	//Iterate through each record that the bucket has.
    for(int i = 0; i < bucket_info->rec_num; i++){     
        rec = (data + sizeof(Bucket_info) + i * sizeof(Record));
		/*Print the record's information when the specific one is found.(Does not return HT_OK 
		in case there are many people with the same id, in other scenarios) */
        if(rec->id == *id){
            printf("NAME:%s ARRAY:%d SURNAME:%s ID:%d CITY:%s\n",rec->name,i, rec->surname, rec->id, rec->city);
            error = 0;
        }
    }

    if(!error)
        return HT_OK;
    
    printf("NO RECORD WITH ID = %d\n",*id);
    return error;
    
}

HT_ErrorCode HashStatistics(char* filename){

    Index_info* index_info;
    Bucket_info* bucket_info;

    BF_Block* block;
	BF_Block_Init(&block);

    void* data;
    int index_desc;

	//OpenIndex to get the index_desc so that the meta data can be obtained
    CALL_BF(HT_OpenIndex(filename, &index_desc));

    index_info = open_files[index_desc];

    printf("\n----------STATISTICS----------\n");
    printf("FILE %s HAS %d BLOCKS/BUCKETS\n", filename, index_info->bucket_num);

    int max = -1;
    int min = index_info->max_rec + 1;
    int sum = 0; //Stores the number of all records inside the the file => matches RECORDS_NUM at the end

	//Iterates through all the buckets available to find the statistics needed
    for(int i = 1; i <= index_info->bucket_num; i++){
        BF_GetBlock(index_desc, i, block);      
        data = BF_Block_GetData(block);
        bucket_info = data;

        BF_Block_SetDirty(block);
        BF_UnpinBlock(block);

        sum += bucket_info->rec_num;

		//Updates minimum records found inside a bucket
        if (bucket_info->rec_num <= min)
            min = bucket_info->rec_num; 

		//Updates maximum records found inside a bucket
        if(bucket_info->rec_num >= max)
            max = bucket_info->rec_num;
    }

    float avg = (float)(sum) / (index_info->bucket_num); //records per bucket

    printf("MAXIMUM NUMBER OF RECORDS: %d\n",max);
    printf("MIN NUMBER OF RECORDS: %d\n",min);
    printf("NUMBER OF RECORDS PER BUCKET: %0.2f\n", avg);
    printf("-------------------------------\n");

    BF_Block_Destroy(&block);
    return HT_OK;
}