#include <merge.h>
#include <stdio.h>
#include "chunk.h"


CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk){
    
}

int CHUNK_GetNext(CHUNK_Iterator *iterator,CHUNK* chunk){

}

int CHUNK_GetIthRecordInChunk(CHUNK* chunk,  int i, Record* record){

}

int CHUNK_UpdateIthRecord(CHUNK* chunk,  int i, Record record){

}

void CHUNK_Print(CHUNK chunk){

}


CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk){

}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator,Record* record){
    
}
