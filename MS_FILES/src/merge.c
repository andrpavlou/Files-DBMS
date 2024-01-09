#include <merge.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define MAX_SIZE 27  // Maximum size for each array
#define MAX_ARR 4   // Maximum number of arrays

void mergeRange(Record **arr, int start, int end, int input_FileDesc, int output_FileDesc, int last_block_updated) {
    Record new_array[MAX_SIZE * MAX_ARR]; // New array to store the merged result
    int temp_indices[MAX_ARR];         // Array to keep track of current indices in each array
    int new_array_index = 0;
    int block_id = last_block_updated;
    int cursor = 0;
    

    // Initialize temp_indices to the start of each array in the range
    for (int i = start; i <= end; ++i) {
        temp_indices[i] = 0;
    }


    while (new_array_index < MAX_SIZE * (end - start + 1)) {
        Record min_val;
        Record min_val2;

        strcpy(min_val.name, "Zzzzzzzzzz");

        int min_index = -1;
        // Find the minimum value among the current elements of the specified range of arrays
        for (int i = start; i <= end; ++i) {

            if (temp_indices[i] < MAX_SIZE && strcmp(arr[i][temp_indices[i]].name, min_val.name) < 0) {
                min_val = arr[i][temp_indices[i]];
                min_index = i;
            }
            if (temp_indices[i] < MAX_SIZE && !strcmp(arr[i][temp_indices[i]].name, min_val.name) && 
            strcmp(arr[i][temp_indices[i]].surname, min_val.surname) < 0){
                min_val = arr[i][temp_indices[i]];
                min_index = i;
            }
        }
        
            // printf(" %d \n", HP_GetRecordCounter(input_FileDesc, block_id));
        if(cursor >= HP_GetRecordCounter(input_FileDesc, block_id)){
            cursor = 0;
            block_id++;
        }
        if(block_id > 56)
            break;

        // printf("%d , %d \n ", cursor, block_id);
        HP_UpdateRecord(output_FileDesc, block_id, cursor, min_val);
        cursor++;
        // Add the minimum value to the new array
        new_array[new_array_index++] = min_val;

        // Move to the next element in the array from which the minimum value was taken
        temp_indices[min_index]++;
    }

    // HP_PrintAllEntries(output_FileDesc);

    // for (int i = 0; i < MAX_SIZE * (end - start + 1); ++i) {
    //     printRecord(new_array[i]);
    // }
    // printf("\n");
}





void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc){
    CHUNK chunk;
    CHUNK first_chunk, last_chunk;

    chunk.from_BlockId = 1;
    chunk.to_BlockId = chunkSize;
    chunk.file_desc = input_FileDesc;
    chunk.recordsInChunk = chunkSize * HP_GetMaxRecordsInBlock(input_FileDesc);
    chunk.blocksInChunk = chunkSize;
    first_chunk = chunk;

    CHUNK_RecordIterator rec_iter[bWay];
    CHUNK_Iterator chunk_it  = CHUNK_CreateIterator(input_FileDesc, chunkSize);
    rec_iter[0] = CHUNK_CreateRecordIterator(&chunk);
    chunk_it.current = chunk.to_BlockId;
    last_chunk = chunk;

    while(last_chunk.to_BlockId != HP_GetIdOfLastBlock(input_FileDesc)){
        int index = 1;
        while (CHUNK_GetNext(&chunk_it, &chunk) == 1 && index < bWay ) {
            chunk_it.current = chunk.to_BlockId;
            rec_iter[index] = CHUNK_CreateRecordIterator(&chunk);
            index ++; 
            last_chunk = chunk;
        }
        
        Record* chunk_recs[bWay];
        for(int j = 0; j < bWay; j++){
            int total_rec = 0;
            for (int i = rec_iter[j].chunk.from_BlockId; i <= rec_iter[j].chunk.to_BlockId; ++i) {
                rec_iter[j].currentBlockId = rec_iter->chunk.to_BlockId;
                total_rec += HP_GetRecordCounter(rec_iter[j].chunk.file_desc, i);
            }
            chunk_recs[j] = malloc(total_rec * sizeof(Record));
        }

        for(int i = 0; i < bWay; i++){
            int curr = 0;
            while(CHUNK_GetIthRecordInChunk(&rec_iter[i].chunk, curr, &chunk_recs[i][curr]) == 0){
                curr ++;
            }
        }

        int end = bWay - 1;
        int divv = last_chunk.to_BlockId - first_chunk.from_BlockId + 1;
        divv = divv % (bWay * chunkSize);

        if(divv != 0)
            end = divv / chunkSize - 1;
        mergeRange(chunk_recs, 0, end, input_FileDesc, output_FileDesc, first_chunk.from_BlockId);


        rec_iter[0].chunk = chunk;
        first_chunk = chunk;
        chunk_it.current = chunk.to_BlockId;
        // for(int i = 0; i < bWay; i++)
            // free(chunk_recs[i]);
    }

    HP_PrintAllEntries(output_FileDesc);
}



