#include <merge.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

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

int nextOutputFile(int* fileCounter){
    char mergedFile[50];
    char tmp[] = "out";
    sprintf(mergedFile, "%s%d.db", tmp, (*fileCounter)++);
    int file_desc = createAndPopulateHeapFile(mergedFile);
    return file_desc;
}



void mergeRange(CHUNK_RecordIterator rec_iter[] , int input_FileDesc, int output_FileDesc, int last_block_updated, int size) {
    int conflicts = 0;  //Stores the number of chunks that have been fully checked. 
    int output_file_cursor = 0; //Current record cursor of new file.
    int block_id = last_block_updated;

    char* max_string = "Zzzzzzzzzzzzz";

    while(conflicts < size && block_id <= HP_GetIdOfLastBlock(output_FileDesc)){ 
        Record current_rec; 
        Record min_rec; //Current alphabetically first record.

        strcpy(min_rec.name, max_string);
        int min_index = -1;

        //Find the minimum record, among the current elements of the array.
        for (int i = 0; i < size; ++i) {
            int eligible = 1;

            //Out of bound chunk, therefore do take its value into consideration.
            if(rec_iter[i].cursor == HP_GetRecordCounter(input_FileDesc, rec_iter[i].currentBlockId) &&
            (rec_iter[i].currentBlockId == rec_iter[i].chunk.to_BlockId)){
                eligible = 0;
            }

            //Finds the new current record.
            HP_GetRecord(input_FileDesc, rec_iter[i].currentBlockId, rec_iter[i].cursor, &current_rec);
            HP_Unpin(input_FileDesc, rec_iter[i].currentBlockId);

            //Comparison between current record, and previous minimum rec (if the current is inside the bounds).
            if(eligible && shouldSwap(&min_rec, &current_rec)){
                min_rec = current_rec;
                min_index = i;
            }
        }
        rec_iter[min_index].cursor++;

        //Update the rec_iter[min_index], cursor and block if needed.
        if(rec_iter[min_index].currentBlockId < rec_iter[min_index].chunk.to_BlockId && 
        rec_iter[min_index].cursor == HP_GetRecordCounter(input_FileDesc, rec_iter[min_index].currentBlockId)){
            rec_iter[min_index].currentBlockId++;
            rec_iter[min_index].cursor = 0;
        //rec_iter[min_index].chunk has been fully iterated.
        }else if(rec_iter[min_index].currentBlockId == rec_iter[min_index].chunk.to_BlockId &&
        rec_iter[min_index].cursor == HP_GetRecordCounter(input_FileDesc, rec_iter[min_index].currentBlockId)){
            conflicts++;            
        }

        //Updates, the output file with the new minimum record, that was previously found.
        int ret = HP_UpdateRecord(output_FileDesc, block_id, output_file_cursor, min_rec);
        HP_Unpin(output_FileDesc, block_id);

        //Error that was caused due to HP_UpdateRecord.
        if(ret == -1){
            printf("THERE WAS AN ERROR, UPDATING THE RECORD\n");
            return;
        }

        output_file_cursor++;
        if(output_file_cursor == HP_GetRecordCounter(input_FileDesc, block_id)){
            output_file_cursor = 0;
            block_id++;
        }        

    }
}


void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc, int* filecounter){
    //File is sorted, with file descriptor = output_FileDesc.
    if(chunkSize == -1){
        (*filecounter) = output_FileDesc;
        return;
    }
    CHUNK chunk;
    chunk_init(&chunk, chunkSize, input_FileDesc);
    CHUNK last_chunk = chunk; //Stores the last chunk.

    CHUNK_RecordIterator rec_iter[bWay];
    CHUNK_Iterator chunk_it  = CHUNK_CreateIterator(input_FileDesc, chunkSize);

    int count = 0, index = 1;
    while(last_chunk.to_BlockId != HP_GetIdOfLastBlock(input_FileDesc)){
        //Manually find the first record iterator.
        rec_iter[0] = CHUNK_CreateRecordIterator(&chunk);
        chunk_it.current = chunk.to_BlockId;
        last_chunk = chunk;
        
        index = 1;
        //Finds the next bWay - 1 record iterator.(Except last one, has less than or equal to bWay - 1).
        while(CHUNK_GetNext(&chunk_it, &chunk) == 1 && index < bWay) {
            last_chunk = chunk;
            rec_iter[index] = CHUNK_CreateRecordIterator(&chunk);
            chunk_it.current = chunk.to_BlockId;
            index ++; 
        }
        mergeRange(rec_iter, input_FileDesc, output_FileDesc, rec_iter[0].chunk.from_BlockId, index);
    }

    int new_chunksize = chunkSize * bWay;
    
    //If chunkSize = id of last block, it means the file has just been fully sorted.
    if(chunkSize == HP_GetIdOfLastBlock(input_FileDesc))
        new_chunksize = -1;

    //new_chunksize > last_id -> one iteration left for the file to become fully sorted.
    if(new_chunksize > HP_GetIdOfLastBlock(input_FileDesc)) 
        new_chunksize = HP_GetIdOfLastBlock(input_FileDesc);
    
    //After each iteration, the output is extracted in a new file.
    int new_file; //New file descriptor for the next iteration.
    if(new_chunksize != -1){
        new_file = nextOutputFile(filecounter);
    }else{
        new_file = input_FileDesc;
    }
    //Recursively call merge function with the new chunk size which is chunkSize * bWay.
    merge(output_FileDesc, new_chunksize, bWay, new_file, filecounter);
}



