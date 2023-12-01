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
void least_significant_bits(char* msb, char* string_key, int gdepth){
	string_key = (string_key + strlen(string_key) - gdepth);
	strncpy(msb, string_key, gdepth + 1);
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
	BF_Block_SetDirty(block);
	BF_UnpinBlock(block);

  	int power = pow(2, depth);

  	index = data;
  	index->global_depth = depth;
	index->file_desc = file_desc;
	index->bucket_num = 0;
	index->last_block_id = 0;
  	index->max_rec = (BF_BLOCK_SIZE - sizeof(Bucket_info)) / (sizeof(Record) + 1);

	index->size = power;

	index->block_ids = malloc(index->size * sizeof(int));

  	BF_Block_Destroy(&block);

	return HT_OK;
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
    BF_Block* block;
    BF_Block_Init(&block);

    int file_desc;
    CALL_BF(BF_OpenFile(fileName, &file_desc));
    CALL_BF(BF_GetBlock(file_desc, 0, block));
	
    void* header = BF_Block_GetData(block);
    Index_info* info;
    info = header;

	BF_Block_Destroy(&block);

	info->file_desc = file_desc;

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

int final_key_index(int id, int bit_num){

	int key = hash_func(id);
	char str_key[32];
	intToBinary(key, str_key);
	char* msb = malloc(bit_num * sizeof(char));
	least_significant_bits(msb, str_key, bit_num);
	int arraykey = stringToInt(msb);
	

	free(msb);
	return arraykey;
}

void printallrecs(Index_info* index){
	
	BF_Block* block;
	BF_Block_Init(&block);
	int sum = 0;
	printf("INDEX SIZE:%d GLOBAL DEPTH:%d\n-----------------\n", index->size, index->global_depth);
	for(int j = 1; j <= index->bucket_num; j++){
		(BF_GetBlock(index->file_desc, j, block));

		void* test = BF_Block_GetData(block);
		Bucket_info* bucktest = test;
		BF_Block_SetDirty(block);
		BF_UnpinBlock(block);
		sum += bucktest->rec_num;
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

	BF_Block_SetDirty(block);
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);
	

	//split buddies
	if(index->global_depth - buck->local_depth == 1 && buck->rec_num == index->max_rec){
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
		HT_InsertEntry(indexDesc, record);
		BF_Block_SetDirty(newblock);
		BF_UnpinBlock(newblock);
		BF_Block_Destroy(&newblock);
		return HT_OK;
	}

	if(index->global_depth > buck->local_depth && index->global_depth - buck->local_depth != 1 && buck->rec_num == index->max_rec){
		BF_Block* newblock;
		BF_Block_Init(&newblock);
	

		CALL_BF(BF_AllocateBlock(index->file_desc, newblock));
		index->bucket_num ++;

		int buddie_index = 0;
		buddie_index = array_key - pow(2, (index->global_depth - 1));

		index->last_block_id ++;
		index->block_ids[array_key] = index->last_block_id;

		if(buddie_index < 0)
			buddie_index = array_key + pow(2, (index->global_depth - 1));


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

		
		HT_InsertEntry(indexDesc, record);
		BF_Block_SetDirty(newblock);
		BF_UnpinBlock(newblock);
		BF_Block_Destroy(&newblock);

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

	if(record.id = 779)
		printf("errorororo");
	return -1;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {

    Index_info* index = open_files[indexDesc]; 
    BF_Block* block;
    Bucket_info* bucket_info;
    Record* rec;
    void* data;
    int error = -1;

    
    if(id == NULL){
        printallrecs(index);
        return HT_OK;
    }

    BF_Block_Init(&block);
    int hash_key = final_key_index(*id ,index->global_depth);
    int block_id = index->block_ids[hash_key];

    CALL_BF( BF_GetBlock(index->file_desc, block_id, block));
    data = BF_Block_GetData(block);
    bucket_info = data;

	BF_Block_SetDirty(block);
	BF_UnpinBlock(block);
	BF_Block_Destroy(&block);


    for(int i = 0; i < bucket_info->rec_num ; i++){     
        rec = (data + sizeof(Bucket_info) + i * sizeof(Record));
        if( rec->id == *id ){
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
    void* data;
    int index_desc;

    BF_Block_Init(&block);
    CALL_BF(HT_OpenIndex(filename, &index_desc));

    index_info = open_files[0];

    printf("\n----------STATISTICS----------\n");
    printf("FILE %s HAS %d BLOCKS/BUCKETS\n", filename, index_info->bucket_num);

    int max = -1;
    int min = index_info->max_rec + 1;
    int sum = 0;

    for(int i = 1; i <= index_info->bucket_num ; i ++){
        
        BF_GetBlock(index_desc, i, block);      
        data = BF_Block_GetData(block);
        bucket_info = data;

        BF_Block_SetDirty(block);
        BF_UnpinBlock(block);

        sum += bucket_info->rec_num;

        if (bucket_info->rec_num <= min)
            min = bucket_info->rec_num; 

        if(bucket_info->rec_num >= max)
            max = bucket_info->rec_num;

    }
    float avg = (float)(sum) / (index_info->bucket_num);

    printf("MAXIMUM NUMBER OF RECORDS: %d\n",max);
    printf("MIN NUMBER OF RECORDS: %d\n",min);
    printf("NUMBER OF RECORDS PER BUCKET: %0.2f\n", avg);
    printf("-------------------------------\n");

    BF_Block_Destroy(&block);
    return HT_OK;
}