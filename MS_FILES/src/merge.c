#include <merge.h>
#include <stdio.h>
#include <stdbool.h>

void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc){
    CHUNK_Iterator chunk_it = CHUNK_CreateIterator(input_FileDesc, chunkSize);
    CHUNK chunk, last_chunk;
    CHUNK_RecordIterator rec_iter[bWay];

    chunk.from_BlockId = 1;
    chunk.to_BlockId = chunkSize;
    chunk.file_desc = input_FileDesc;
    chunk.recordsInChunk = chunkSize * HP_GetMaxRecordsInBlock(input_FileDesc);
    chunk.blocksInChunk = chunkSize;    
    rec_iter[0] = CHUNK_CreateRecordIterator(&chunk);

    for(int i = 1; i < bWay; i++){
        CHUNK_GetNext(&chunk_it, &chunk);   
        rec_iter[i] = CHUNK_CreateRecordIterator(&chunk);
        last_chunk = chunk;
    }   

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




    //last_chunk = 2 -> arr[2]->chunk4 = get_next(arr[2]) last_chunk = chunk

    //arr -> chunk1 chunk2 chunk3
    //arr_chunk                        arr_block

    //chunk1 -> 1-3  block1   =>        block2
    //chunk2 -> 4-6  block6   =>        block6 -> id6
    //chunk3 -> 7-9  block7   =>        block7

    //arr_block[1] - > empty => arr_chunk[1]
    //arr_block[1] id == arr_chunk[1]->last_id ???


    
    //arr_block[3] => merge to kathe block -> temp


    //temp_block1 = full, block4 ->empty
    //temp_block2 = full, block5 ->empty 
    //temp_block3 = full, block6 ->empty