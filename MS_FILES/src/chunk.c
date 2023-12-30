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
    //-----egine allagi prin itan iterate->current + ....
    if (iterator->current < iterator->lastBlocksID) {
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
        
        chunk->recordsInChunk = num_rec;
        chunk->blocksInChunk = iterator->blocksInChunk;

        // Move to the next chunk
        iterator->current = chunk->to_BlockId + 1;

        return 1; // Success
    } 
    else {
        return -1; // No more chunks to iterate
        }
}

int CHUNK_GetIthRecordInChunk(CHUNK* chunk, int i, Record* record) {
   
    // Check if the provided index is within the range of records in the chunk
    if (i >= 0 && i < chunk->recordsInChunk) {
        // Calculate the block and record index within the chunk
        
        int blockIndex = i / HP_GetRecordCounter(chunk->file_desc, chunk->from_BlockId);
        int recordIndexInBlock = i % HP_GetRecordCounter(chunk->file_desc, chunk->from_BlockId);

        // Calculate the actual block ID based on the starting block ID of the chunk
        int blockId = chunk->from_BlockId + blockIndex;

        // Assuming a function to retrieve the record from a block, replace accordingly
        if (HP_GetRecord(chunk->file_desc, blockId, recordIndexInBlock, record) == 0) {
            return 0; // Success
        } else {
            return -1; // Error in retrieving the record from the block
        }
    } 
    else {
        return -1; // Index out of bounds
    }

}

int CHUNK_UpdateIthRecord(CHUNK* chunk, int i, Record record) {
   
    // Check if the provided index is within the range of records in the chunk
    if (i >= 0 && i < chunk->recordsInChunk) {
        // Calculate the block and record index within the chunk
        int blockIndex = i / HP_GetRecordCounter(chunk->file_desc, chunk->from_BlockId);
        int recordIndexInBlock = i % HP_GetRecordCounter(chunk->file_desc, chunk->from_BlockId);

        // Calculate the actual block ID based on the starting block ID of the chunk
        int blockId = chunk->from_BlockId + blockIndex;

        // Assuming a function to update the record in a block, replace accordingly
        if (HP_UpdateRecord(chunk->file_desc, blockId, recordIndexInBlock, record) == 0) {
            return 0; // Success
        } else {
            return -1; // Error in updating the record in the block
        }
    } else {
        return -1; // Index out of bounds
    }
}

void CHUNK_Print(CHUNK chunk) {
    
    // Loop through each record in the chunk and print it
    Record record;
    for (int i = 0; i < chunk.recordsInChunk; ++i) {
        
        if (CHUNK_GetIthRecordInChunk(&chunk, i, &record) == 0)
            printRecord(record);

        else 
            printf("Error retrieving record %d\n", i);
    }
}


CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk) {
    CHUNK_RecordIterator iterator;
    iterator.chunk = *chunk;
    iterator.currentBlockId = chunk->from_BlockId;
    iterator.cursor = 0;
    return iterator;
}

int CHUNK_RecordIterator_GetNext(CHUNK_RecordIterator *iterator, Record *record) {
    // Calculate the total number of records in the chunk
    int totalRecords = 0;
    for (int i = iterator->chunk.from_BlockId; i <= iterator->chunk.to_BlockId; ++i) {
        totalRecords += HP_GetRecordCounter(iterator->chunk.file_desc, i);
    }

    // Check if there are more records to iterate
    if (iterator->cursor < totalRecords) {
        // Retrieve the next record using the existing CHUNK_GetIthRecordInChunk function
        if (CHUNK_GetIthRecordInChunk(&(iterator->chunk), iterator->cursor, record) == 0) {
            // Move the cursor to the next record
            ++iterator->cursor;
            return 1; // Success
        } 
        else {
            return -1; // Error in retrieving the record
        }
    
    } else {
        return 0; // No more records to iterate
    }
}