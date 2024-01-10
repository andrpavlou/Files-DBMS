#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merge.h"

#define FILE_NAME "data.db"
#define OUT_NAME "out"

int main() {
    int chunkSize = 5;
    int bWay = 10;
    BF_Init(LRU);
    int file_desc = createAndPopulateHeapFile(FILE_NAME);
    int file_desc2 = createAndPopulateHeapFile(OUT_NAME);
    int filecounter = 0;

    sort_FileInChunks(file_desc, chunkSize);
    merge(file_desc, chunkSize, bWay, file_desc2, &filecounter);

    HP_PrintAllEntries(filecounter);
}
