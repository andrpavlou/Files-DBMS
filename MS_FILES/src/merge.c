#include <merge.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define MAX_SIZE 27  // Maximum size for each array
#define MAX_ARR 4   // Maximum number of arrays

void mergeRange(Record **arr, int start, int end, int output_FileDesc) {
    Record new_array[MAX_SIZE * MAX_ARR]; // New array to store the merged result
    int temp_indices[MAX_ARR];         // Array to keep track of current indices in each array
    int new_array_index = 0;
    int block_id = 1;
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

        if(cursor >= HP_GetMaxRecordsInBlock(output_FileDesc)){
            cursor = 0;
            block_id++;
        }
        // printf("%d , %d \n ", cursor, block_id);
        HP_UpdateRecord(output_FileDesc, block_id, cursor, min_val);
        cursor++;
        // Add the minimum value to the new array
        new_array[new_array_index++] = min_val;

        // Move to the next element in the array from which the minimum value was taken
        temp_indices[min_index]++;
    }

    HP_PrintAllEntries(output_FileDesc);

    // for (int i = 0; i < MAX_SIZE * (end - start + 1); ++i) {
    //     printRecord(new_array[i]);
    // }
    printf("\n");
}





void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc){
    CHUNK chunk;
    CHUNK last_chunk;

    chunk.from_BlockId = 1;
    chunk.to_BlockId = chunkSize;
    chunk.file_desc = input_FileDesc;
    chunk.recordsInChunk = chunkSize * HP_GetMaxRecordsInBlock(input_FileDesc);
    chunk.blocksInChunk = chunkSize;


    CHUNK_RecordIterator rec_iter[bWay];
    CHUNK_Iterator chunk_it  = CHUNK_CreateIterator(input_FileDesc, chunkSize);
    rec_iter[0] = CHUNK_CreateRecordIterator(&chunk);
    chunk_it.current = chunk.to_BlockId;

    int index = 1;
    while (CHUNK_GetNext(&chunk_it, &chunk) == 1 && index < bWay ) {
        chunk_it.current = chunk.to_BlockId;
        rec_iter[index] = CHUNK_CreateRecordIterator(&chunk);
        index ++; 
    }
    
    Record* chunk_recs[bWay];
    int full_total = 0;
    int total_rec = 0;
    for(int j = 0; j < bWay; j++){
        for (int i = rec_iter[j].chunk.from_BlockId; i <= rec_iter[j].chunk.to_BlockId; ++i) {
            rec_iter[j].currentBlockId = rec_iter->chunk.to_BlockId;
            total_rec += HP_GetRecordCounter(rec_iter[j].chunk.file_desc, i);
        }
        full_total += total_rec;

        chunk_recs[j] = malloc(total_rec * sizeof(Record));
        total_rec = 0;
    }

    int curr = 0;
    for(int i = 0; i < bWay; i++){
        while(CHUNK_GetIthRecordInChunk(&rec_iter[i].chunk, curr, &chunk_recs[i][curr]) == 0){
            curr ++;
        }
        curr = 0;
    }

    // for(int i = 0; i < bWay; i++){
    //     CHUNK_Print(rec_iter[i].chunk);
    //     printf("\n----------------\n");
    // }
    mergeRange(chunk_recs, 0, 3, output_FileDesc);
        //IMPLEMENT MERGE SORT
}





/* #include <stdio.h>

#define MAX_SIZE 3 // Maximum size for each array
#define MAX_ARR 10   // Maximum number of arrays

void mergeRange(int arr[][MAX_SIZE], int start, int end) {
    int new_array[MAX_SIZE * MAX_ARR]; // New array to store the merged result
    int temp_indices[MAX_ARR];         // Array to keep track of current indices in each array
    int new_array_index = 0;
    // Initialize temp_indices to the start of each array in the range
    for (int i = start; i <= end; ++i) {
        temp_indices[i] = 0;
    }

    while (new_array_index < MAX_SIZE) {
        int min_val = __INT_MAX__;
        int min_index = -1;

        // Find the minimum value among the current elements of the specified range of arrays
        for (int i = start; i <= end; ++i) {
            if (temp_indices[i] < MAX_SIZE && arr[i][temp_indices[i]] < min_val && arr[i][temp_indices[i]] != 0) {
                min_val = arr[i][temp_indices[i]];
                min_index = i;
            }
        }

        // Add the minimum value to the new array
        new_array[new_array_index++] = min_val;

        // Move to the next element in the array from which the minimum value was taken
        temp_indices[min_index]++;
    }

    // Print the new sorted array
    printf("New sorted array based on arrays from %d to %d:\n", start, end);
    for (int i = 0; i < MAX_SIZE; i++) {
        printf("%d ", new_array[i]);
    }
    printf("\n");
}

int main() {
    int arrays[MAX_ARR][MAX_SIZE] = {
        {9},
        {8},
        {10},
        // Add more arrays here if needed
    };

    int num_arrays = 3; // Number of arrays
    int start_index = 0; // Start index of the range
    int end_index = 2; // End index of the range

    mergeRange(arrays, start_index, end_index);

    return 0;
}
*/




    