#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "sort.h"
#include "merge.h"
#include "chunk.h"

// Function to determine if two records should be swapped during sorting
bool shouldSwap(struct Record *rec1, struct Record *rec2) {
    // Compare based on the 'name' field
    int nameComparison = strcmp(rec1->name, rec2->name);

    // If names are the same, compare based on the 'surname' field
    if (nameComparison == 0) {
        return strcmp(rec1->surname, rec2->surname) > 0;
    }

    // Otherwise, return true if the first name is greater than the second name
    return nameComparison > 0;
}


void sort_FileInChunks(int file_desc, int numBlocksInChunk) {
    // Assuming CHUNK_GetNext is a function that retrieves the next chunk in the file
    CHUNK chunk;
    chunk_init(&chunk, numBlocksInChunk, file_desc);
    sort_Chunk(&chunk);
    
    CHUNK_Iterator iterator = CHUNK_CreateIterator(file_desc, numBlocksInChunk);
    iterator.current = chunk.to_BlockId;


    // Iterate through each chunk
    while (CHUNK_GetNext(&iterator, &chunk) == 1) {
        // printf("%d %d \n", chunk.from_BlockId, chunk.to_BlockId);
        iterator.current = chunk.to_BlockId;
        // Sort records within the current chunk
        sort_Chunk(&chunk);
    }
}

void sort_Chunk(CHUNK* chunk) {
    int numRecords = chunk->recordsInChunk;

    for (int i = 0; i < numRecords - 1; i++) {
        for (int j = 0; j < numRecords - i - 1; j++) {
            Record record1, record2;

            // Retrieve records at positions j and j+1
            if (CHUNK_GetIthRecordInChunk(chunk, j, &record1) == 0 &&
                CHUNK_GetIthRecordInChunk(chunk, j + 1, &record2) == 0) {
                // Compare records and swap if necessary
                if (shouldSwap(&record1, &record2) > 0) {
                    // Swap records using CHUNK_UpdateIthRecord
                    if (CHUNK_UpdateIthRecord(chunk, j, record2) == 0 &&
                        CHUNK_UpdateIthRecord(chunk, j + 1, record1) == 0) {
                        // Swap successful
                    } else {
                        return;
                    }
                }
            } else {
                    return;
            }
        }
    }
}
