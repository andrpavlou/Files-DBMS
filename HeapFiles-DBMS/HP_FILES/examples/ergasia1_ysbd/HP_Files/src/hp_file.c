#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    return -1;              \
  }                         \
}

typedef struct {
    int num_rec;       //Number of records in each block.
    void* next_block; //Pointer of the next block.(Not used becasue it is not needed in HeapFiles)
} HP_block_info;      

HP_block_info binfo = {
  .num_rec = 0, 
  .next_block = NULL 
};

int HP_CreateFile(char *fileName){
  //File creation.
  int file_desc;
  CALL_BF(BF_CreateFile(fileName));
  CALL_BF(BF_OpenFile(fileName, &file_desc));

  //First block initialization.
  BF_Block* block;
  BF_Block_Init(&block);
  CALL_BF(BF_AllocateBlock(file_desc, block));
  
  void* data;
  data = BF_Block_GetData(block);

  //Insert file data in the block 0.
  HP_info info;
  HP_info* pinfo = &info;
  pinfo = data;

  int block_num, info_size, rec_size;
  CALL_BF(BF_GetBlockCounter(file_desc, &block_num));
  
  pinfo->id = block_num - 1;                                                  //The id of the last block inserted.   
  pinfo->max_rec = (BF_BLOCK_SIZE - sizeof(HP_info) )/ (sizeof(Record) + 1);  //Maximum records allowed in a block.

  //Close the file.
  CALL_BF(BF_CloseFile(file_desc));
  //Free memory of block structure.
  BF_Block_Destroy(&block);

  return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc){
  HP_info info;
  HP_info* pinfo = &info;

  BF_Block *block;
  BF_Block_Init(&block);

  void* data;

  BF_OpenFile(fileName , file_desc);

  //File information needed is stored in block 0. 
  BF_GetBlock(*file_desc, 0, block);
  
  //Access to the data of block 0.
  data = BF_Block_GetData(block);
  pinfo = data;

  BF_Block_Destroy(&block);
  
  return pinfo;
}

int HP_CloseFile(int file_desc,HP_info* hp_info ){
  BF_Block  *block;
  BF_Block_Init(&block);

  int id1 = hp_info->id, block_num;
  CALL_BF(BF_GetBlockCounter(file_desc, &block_num));

  //Set dirty and unpin every block.
  for( int i = 0 ; i < block_num; i++ ){
    CALL_BF(BF_GetBlock(file_desc ,i  ,block));
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
  }

  //Free memory of block structure, close the file, write everything on the disk.
  //HP_info structure is also freed by the library.
  
  BF_Block_Destroy(&block);
  CALL_BF(BF_CloseFile(file_desc));
  CALL_BF(BF_Close());

  return 0;
}

int HP_InsertEntry(int file_desc,HP_info* hp_info, Record record){
  HP_block_info binfo;
  HP_block_info* pbinfo = &binfo;
  BF_Block* block;

  int block_num, offsetbf = (hp_info->max_rec) * sizeof(Record);
  void* data;
  
  BF_Block_Init(&block);

  //Only block 0 is available so we have to create a new one.
  if(hp_info->id == 0){
    hp_info->id ++;

    CALL_BF(BF_AllocateBlock(file_desc , block));
    data = BF_Block_GetData(block);

    //At the start of the data of each block (except 0) we store the information of each block. 
    pbinfo = data;
    pbinfo->num_rec = 1;

    //Insert the record after block data.
    Record* rec = (data + sizeof(HP_block_info));
    *rec = record;

    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));

    return hp_info->id;
  }
  //More blocks are available, not only 0.

  //Access to the last block to get block data.
  CALL_BF(BF_GetBlock(file_desc , hp_info->id, block));

  data = BF_Block_GetData(block);
  pbinfo = data;

  //There is space in the last block so the record can be inserted after the last block.
  if(pbinfo->num_rec < hp_info->max_rec){
    Record* rec;
    //rec has the memory address of the end of the last record.
    rec = (data+ sizeof(HP_block_info) + sizeof(Record) * pbinfo->num_rec);
    *rec = record;
    pbinfo->num_rec ++;

    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    
    return hp_info->id;
  }
  //There is no space in the last block so a new one must be allocated.
  else{
    CALL_BF(BF_GetBlock(file_desc, hp_info->id, block));
    BF_UnpinBlock(block);

    hp_info->id ++;

    CALL_BF(BF_AllocateBlock(file_desc , block));
    data = BF_Block_GetData(block);

    //Insert block information of the new block.
    pbinfo = data;
    pbinfo->num_rec = 1;
    
    //Insert the record after block data.
    Record* rec = (data + sizeof(HP_block_info));
    *rec = record;

    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));

    return hp_info->id;
  } 
  
  return -1;
}
int HP_GetAllEntries(int file_desc,HP_info* hp_info, int value){    
  BF_Block* block;
  BF_Block_Init(&block);


  HP_block_info block_info;
  HP_block_info* pblock_info = &block_info; 

  int last = -1; //Number of blocks that were iterated untill "value" was found in a record.
  int block_num = hp_info->id; //Last block's id.

  void* data;
  
  //Iterate every block except 0, because it does not contain any records.
  for(int i = 1; i <= block_num ; i++){
    CALL_BF(BF_GetBlock(file_desc, i, block));

    //Data of each block.
    data = BF_Block_GetData(block);    
    pblock_info = data;            
   
    int rec_num = pblock_info->num_rec;  //Number of records that block[i] has.
    
    //Iterate every record of block i, until the record is found.
    for(int j = 0; j < rec_num; j++){
      //Each j represeants (j + 1) record.
      //Records start after (HP_block_info structure).     
      Record* next_rec = (data + sizeof(HP_block_info)+ j * sizeof(Record)); 
      //Search if a record matches the given id.
      if(next_rec->id == value){    
        printRecord(*next_rec); 
        //Number of blocks that were read.
        last = i; 
        return last;
      }
    }
    CALL_BF(BF_UnpinBlock(block));
  }
  //Returns -1 if the id was not found.
  return last;
}