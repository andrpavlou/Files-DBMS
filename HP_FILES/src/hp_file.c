#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

typedef struct {
    int num_rec;
    void* next_block;
} HP_block_info;

HP_block_info hpbi = {
  .num_rec = 0,
  .next_block = NULL
};


#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    return -1;              \
  }                         \
}

int HP_CreateFile(char *fileName){

  BF_Block* block;
  void* data;
  
  int fid;
  CALL_BF(BF_CreateFile(fileName));

  BF_Block_Init(&block);

  CALL_BF(BF_OpenFile(fileName , &fid))
  CALL_BF(BF_AllocateBlock(fid , block));

  data = BF_Block_GetData(block);
  HP_info info;
  HP_info* pinfo = &info;
  pinfo = data;


  int block_num ;
  BF_GetBlockCounter( fid , &block_num );
  
  pinfo->id = block_num - 1; 
  pinfo->max_rec = (BF_BLOCK_SIZE - sizeof(HP_block_info))/sizeof(Record);


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

  BF_GetBlock(*file_desc , 0 , block);
  

  data = BF_Block_GetData(block);
  pinfo = data;

  BF_Block_Destroy(&block);
  return pinfo;
}


int HP_CloseFile(int file_desc,HP_info* hp_info){
  BF_Block* block;
  int id = hp_info->id;

  int block_num;
  BF_Block_Init(&block);
  BF_GetBlockCounter(file_desc, &block_num);

  for(int i = 0; i < block_num; i++){
    CALL_BF(BF_GetBlock(file_desc, i, block));
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
  }
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
  //Access to the last block to check block data.
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
    CALL_BF(BF_GetBlock(file_desc, hp_info->id, block);)
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
    return -1;
}

