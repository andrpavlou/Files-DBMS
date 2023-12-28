#include <merge.h>
#include <stdio.h>
#include "chunk.h"

CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk) {
    CHUNK_Iterator iterator;

    // Initialize the iterator with the provided values
    iterator.file_desc = fileDesc;
    iterator.current = 1;  // Starting from block 1
    iterator.lastBlocksID = HP_GetIdOfLastBlock(fileDesc);  // Assuming blocks are numbered starting from 1
    iterator.blocksInChunk = blocksInChunk;

    return iterator;
}

int CHUNK_GetNext(CHUNK_Iterator *iterator, CHUNK *chunk) {
    
    // Check if there are more chunks to iterate
    if (iterator->current + iterator->blocksInChunk <= iterator->lastBlocksID) {
        // Populate the CHUNK structure with relevant data
        chunk->file_desc = iterator->file_desc;
        int d = (iterator->current/iterator->blocksInChunk);
        int rem = (iterator->current % iterator->blocksInChunk);

        if(rem == 0)
            d--;
            
        chunk->from_BlockId = (d * iterator->blocksInChunk) + iterator->blocksInChunk + 1 ;

        int toBlock = chunk->from_BlockId + iterator->blocksInChunk - 1;
        if (toBlock <= iterator->lastBlocksID)
            chunk->to_BlockId = chunk->from_BlockId + iterator->blocksInChunk - 1;
        else
            chunk->to_BlockId = iterator->lastBlocksID; 
       
        int num_rec = 0;
        for(int i = chunk->from_BlockId; i <= chunk->to_BlockId; i ++){
            num_rec += HP_GetRecordCounter(iterator->file_desc, i);
        }
        chunk->recordsInChunk = num_rec;  // Replace with the actual count of records
        chunk->blocksInChunk = iterator->blocksInChunk;

        // Move to the next chunk
        iterator->current = chunk->to_BlockId + 1;

        return 1; // Success
    } else {
        return 0; // No more chunks to iterate
    }
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
