#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merge.h"

#define RECORDS_NUM 500 // you can change it if you want
#define FILE_NAME "data.db"
#define OUT_NAME "out"

int createAndPopulateHeapFile(char* filename);

void sortPhase(int file_desc,int chunkSize);

void mergePhases(int inputFileDesc,int chunkSize,int bWay, int* fileCounter);

int nextOutputFile(int* fileCounter);


int main() {
    int chunkSize = 3;
    int bWay = 4;
    int fileIterator;
    BF_Init(LRU);
    int file_desc = createAndPopulateHeapFile(FILE_NAME);

    Record rec;
    CHUNK chunk;
    CHUNK_Iterator ci = CHUNK_CreateIterator(file_desc, chunkSize);
  
    chunk.from_BlockId = 1;
    chunk.to_BlockId = chunkSize;
    chunk.file_desc = file_desc;
    chunk.recordsInChunk = chunkSize * 9;
    chunk.blocksInChunk = chunkSize;




    sort_FileInChunks(file_desc, chunkSize);
    CHUNK_Print(chunk);
    printf("--------------------------\n");
    while(CHUNK_GetNext(&ci, &chunk) == 1){
        ci.current = chunk.to_BlockId;
        CHUNK_Print(chunk);
        printf("--------------------------\n");
    }

    int file_desc2 = HP_CreateFile(OUT_NAME);


    merge(file_desc, chunkSize, bWay, file_desc2);

    //Out of bound
    // CHUNK_GetIthRecordInChunk(&chunk, 18, &rec);
    // printf("RECORD IN POSITION THAT DOES NOT EXIT:");
    // printRecord(rec);
    // printf("\n");
  
    // //Starting from 0 to recordsInChunk - 1
    // printf("RECORD IN 2nd POS:");
    // printRecord(rec);
    // printf("\n");

    // Record newrec = randomRecord();
    // printf("NEW RECORD:");
    // printRecord(newrec);
    // printf("\n");

    // CHUNK_UpdateIthRecord(&chunk, 2, newrec);
    // CHUNK_GetIthRecordInChunk(&chunk, 2, &rec);
    // printf("UPDATED_RECORD IN 2nd POS:");
    // printRecord(rec);
    // printf("\n");



    // CHUNK_Print(chunk);
    // sort_Chunk(&chunk);
    // printf("-------AFTER:-------\n");
    // CHUNK_Print(chunk);

    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);
    // CHUNK_GetNext(&ci, &chunk);
    // CHUNK_Print(chunk);



    // CHUNK_GetIthRecordInChunk(&chunk, 2, &rec);
    // printRecord(rec);

    // sortPhase(file_desc,chunkSize);  

    // mergePhases(file_desc,chunkSize,bWay,&fileIterator);
}

int createAndPopulateHeapFile(char* filename){
    HP_CreateFile(filename);
  
    int file_desc;
    HP_OpenFile(filename, &file_desc);

    Record record;
    srand(12569874);
    for (int id = 0; id < RECORDS_NUM; ++id){
        record = randomRecord();
        HP_InsertEntry(file_desc, record);
    }
  return file_desc;
}

/*Performs the sorting phase of external merge sort algorithm on a file specified by 'file_desc', using chunks of size 'chunkSize'*/
void sortPhase(int file_desc,int chunkSize){ 
    sort_FileInChunks( file_desc, chunkSize);
}

/* Performs the merge phase of the external merge sort algorithm  using chunks of size 'chunkSize' and 'bWay' merging. The merge phase may be performed in more than one cycles.*/
void mergePhases(int inputFileDesc,int chunkSize,int bWay, int* fileCounter){
    int oututFileDesc;
    while(chunkSize<=HP_GetIdOfLastBlock(inputFileDesc)){
        oututFileDesc =   nextOutputFile(fileCounter);
        merge(inputFileDesc, chunkSize, bWay, oututFileDesc );
        HP_CloseFile(inputFileDesc);
        chunkSize*=bWay;
        inputFileDesc = oututFileDesc;
    }
    HP_CloseFile(oututFileDesc);
}

/*Creates a sequence of heap files: out0.db, out1.db, ... and returns for each heap file its corresponding file descriptor. */
int nextOutputFile(int* fileCounter){
    char mergedFile[50];
    char tmp[] = "out";
    sprintf(mergedFile, "%s%d.db", tmp, (*fileCounter)++);
    int file_desc;
    HP_CreateFile(mergedFile);
    HP_OpenFile(mergedFile, &file_desc);
    return file_desc;
}
