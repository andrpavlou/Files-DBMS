#include <merge.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define MAX_SIZE 27  // Maximum size for each array
#define MAX_ARR 4   // Maximum number of arrays

void mergeRange(CHUNK_RecordIterator rec_iter[] , int input_FileDesc, int output_FileDesc, int last_block_updated, int size) {
    Record rec_array[size]; // New array to store the merged result
    int new_array_index = 0;
    int cursor = 0;
    int block_id = last_block_updated;
    int counter = 0;

    for(int i = 0; i < size; i++){
        HP_GetRecord(input_FileDesc, rec_iter[i].currentBlockId, rec_iter[i].cursor, &rec_array[i]);
    }

    BF_Block* block;
    BF_Block_Init(&block);
    while (counter < size && block_id <= HP_GetIdOfLastBlock(output_FileDesc)){ 
        Record min_val;
        strcpy(min_val.name, "Zzzzzzzzzz");
        int min_index = -1;

        // Find the minimum value among the current elements of the specified range of arrays
        for (int i = 0; i < 4; ++i) {
            if (strcmp(rec_array[i].name, min_val.name) < 0) {
                min_val = rec_array[i];
                min_index = i;
            }
            if (!strcmp(rec_array[i].name, min_val.name) && 
            strcmp(rec_array[i].surname, min_val.surname) < 0){
                min_val = rec_array[i];
                min_index = i;
            }
        }
        rec_iter[min_index].cursor++;


        if(rec_iter[min_index].cursor < HP_GetRecordCounter(input_FileDesc, rec_iter[min_index].currentBlockId)){
            HP_GetRecord(input_FileDesc, rec_iter[min_index].currentBlockId, rec_iter[min_index].cursor, &rec_array[min_index]);
        }else if(rec_iter[min_index].currentBlockId < rec_iter[min_index].chunk.to_BlockId){
            rec_iter[min_index].currentBlockId++;
            rec_iter[min_index].cursor = 0;
            HP_GetRecord(input_FileDesc, rec_iter[min_index].currentBlockId, rec_iter[min_index].cursor, &rec_array[min_index]);
        }else{
            // rec_iter[min_index].currentBlockId = -1;
            strcpy(rec_array[min_index].name,"Zzzzzzzzzzzzz");
            strcpy(rec_array[min_index].surname,"Zzzzzzzzzzzzz");
            counter ++;
        }

        int ret = HP_UpdateRecord(output_FileDesc, block_id, cursor, min_val);
        
        BF_GetBlock(output_FileDesc, block_id, block);

        BF_Block_SetDirty(block);
        BF_UnpinBlock(block);

        strcpy(min_val.name, "Zzzzzzzzzz");

        cursor++;
        if(cursor == 9){
            cursor = 0;
            block_id++;
        }
    }
}





void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc){


    if(chunkSize > HP_GetIdOfLastBlock(input_FileDesc))
        return;
 
    int max_rec = chunkSize * HP_GetMaxRecordsInBlock(input_FileDesc);
    CHUNK chunk;
    CHUNK first_chunk, last_chunk;

    chunk.from_BlockId = 1;
    chunk.to_BlockId = chunkSize;
    chunk.file_desc = input_FileDesc;
    chunk.recordsInChunk = max_rec;
    chunk.blocksInChunk = chunkSize;
    first_chunk = chunk;

    CHUNK_RecordIterator rec_iter[bWay];
    CHUNK_Iterator chunk_it  = CHUNK_CreateIterator(input_FileDesc, chunkSize);
    rec_iter[0] = CHUNK_CreateRecordIterator(&chunk);
    chunk_it.current = chunk.to_BlockId;
    last_chunk = chunk;
    
    int count = 0, index = 1;
    while(last_chunk.to_BlockId != HP_GetIdOfLastBlock(input_FileDesc)){
        index = 1;
        while (CHUNK_GetNext(&chunk_it, &chunk) == 1 && index < bWay ) {
            chunk_it.current = chunk.to_BlockId;
            rec_iter[index] = CHUNK_CreateRecordIterator(&chunk);
            index ++; 
            last_chunk = chunk;
        }

        mergeRange(rec_iter, input_FileDesc, output_FileDesc, rec_iter[0].chunk.from_BlockId, index);
        rec_iter[0] = CHUNK_CreateRecordIterator(&chunk);
        first_chunk = chunk;
        chunk_it.current = chunk.to_BlockId;
    }
    int new_chunksize = chunkSize * 2;
    
    // for(int i = 0; i <=0; i++)

    // if(new_chunksize > HP_GetIdOfLastBlock(input_FileDesc)) 
    //     new_chunksize = HP_GetIdOfLastBlock(input_FileDesc);
    // printf(" %d ", new_chunksize);
    // merge(input_FileDesc, new_chunksize, bWay, output_FileDesc);

    // HP_PrintAllEntries(output_FileDesc);
}



