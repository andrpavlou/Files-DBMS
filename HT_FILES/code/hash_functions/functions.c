#include "../include/hash_file.h"


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

//Prints all records of a file
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

