#include <merge.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define MAX_SIZE 27  // Maximum size for each array
#define MAX_ARR 4   // Maximum number of arrays

void mergeRange(Record **arr, int start, int end) {
    Record new_array[MAX_SIZE * MAX_ARR]; // New array to store the merged result
    int temp_indices[MAX_ARR];         // Array to keep track of current indices in each array
    int new_array_index = 0;

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
            strcmp(arr[i][temp_indices[i]].surname, min_val.surname) < 1){
                min_val = arr[i][temp_indices[i]];
                min_index = i;
            }
        }

        // Add the minimum value to the new array
        new_array[new_array_index++] = min_val;

        // Move to the next element in the array from which the minimum value was taken
        temp_indices[min_index]++;
    }
     for (int i = 0; i < MAX_SIZE * (end - start + 1); ++i) {
        printRecord(new_array[i]);
    }
    printf("\n");
}





void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc){
    CHUNK chunk;
    CHUNK last_chunk;

    chunk.from_BlockId = 1;
    chunk.to_BlockId = chunkSize;
    chunk.file_desc = input_FileDesc;
    chunk.recordsInChunk = chunkSize * 9;
    chunk.blocksInChunk = chunkSize;


    CHUNK_RecordIterator rec_iter[bWay];
    CHUNK_Iterator chunk_it  = CHUNK_CreateIterator(input_FileDesc, bWay);
    rec_iter[0] = CHUNK_CreateRecordIterator(&chunk);
    


    for(int i = 1; i < bWay; i++){
        chunk_it.current = chunk.to_BlockId;
        rec_iter[i] = CHUNK_CreateRecordIterator(&chunk);
        CHUNK_GetNext(&chunk_it, &chunk); 
        last_chunk = chunk;
    }   


    Record* chunk_recs[bWay];
    int full_total = 0;
    int total_rec = 0;
    for(int j = 0; j < bWay; j++){
        for (int i = rec_iter[j].chunk.from_BlockId; i <= rec_iter[j].chunk.to_BlockId; ++i) {
            printf(" %d  %d \n", rec_iter[j].chunk.from_BlockId, rec_iter[j].chunk.to_BlockId);
            total_rec += HP_GetRecordCounter(rec_iter[j].chunk.file_desc, i);
        }
        full_total += total_rec;
        chunk_recs[j] = malloc(total_rec * sizeof(Record));
        total_rec = 0;
    }

    int curr = 0;
    for(int i = 0; i < bWay; i++){
        while(CHUNK_RecordIterator_GetNext(&rec_iter[i], &chunk_recs[i][curr]) == 1){
            curr ++;
        }
        curr = 0;
    }
    // printRecord(chunk_recs[1][30]);
    // mergeRange(chunk_recs, 0, 3);
    // for(int i = 0; i < bWay; i++){
    //     for(int j = 0; j < 27; j++){
    //         printRecord(chunk_recs[i][j]);
    //     }
    //     printf("\n\n");
    // }


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




    