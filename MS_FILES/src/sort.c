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

void sort_FileInChunks(int file_desc, int numBlocksInChunk){

}

void sort_Chunk(CHUNK* chunk){

}