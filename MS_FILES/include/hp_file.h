#ifndef HP_FILE_H
#define HP_FILE_H
#include <stddef.h>

#include "record.h"
#include "bf.h"

typedef struct HP_info{
    int lastBlockId;
    int totalRecords;
    int blockCapacity;
} HP_info;

extern struct HP_info openFiles[20]; 

/*The function HP_CreateFile is used to create and appropriately initialize an empty heap file with the given fileName. If the execution is successful, it returns 0; otherwise, it returns -1.*/
int HP_CreateFile(char *fileName);

/* The function HP_OpenFile opens the file with the name filename. The variable *file_desc refers to the opening identifier of this file as derived from BF_OpenFile.*/
int HP_OpenFile(char *fileName, int *file_desc);

/* The function HP_CloseFile closes the file identified by the descriptor file_desc. If the operation is successful, it returns 0; otherwise, it returns -1.*/
int HP_CloseFile(int file_desc);

/* The function HP_InsertEntry is used to insert a record into the heap file. The identifier for the file is file_desc, and the record to be inserted is specified by the record structure. If the operation is successful, it returns 1; otherwise, it returns -1.*/
int HP_InsertEntry(int file_desc, Record record);

/* The function HP_GetRecord is designed to retrieve a record from a heap file specified by the file descriptor file_desc. It takes three parameters: blockId, which indicates the block from which to retrieve the record, cursor, which specifies the position of the record within that block, and record, which is a pointer to a Record structure. The retrieved record will be stored in the memory location pointed to by the record parameter.*/
int HP_GetRecord( int file_desc, int blockId, int cursor, Record* record);

/* The function HP_UpdateRecord updates or sets a record in a heap file specified by the file descriptor file_desc. It takes four parameters: blockId, which indicates the block where the record will be updated, cursor, which specifies the position of the record within that block, and record, which is the new data that will replace the existing record at the specified location. If the operation is successful, the function returns 1; otherwise, it returns -1.*/
int HP_UpdateRecord(int file_desc, int blockId, int cursor,Record record);

/* The function HP_Unpin is designed to release the block identified by blockId in the heap file associated with the descriptor file_desc. If the unpin is successful, it returns 0; otherwise, it returns -1.*/
int HP_Unpin(int file_desc, int blockId);

// Prints all entries(records) stored in the heap file.
int HP_PrintAllEntries(int file_desc);

// Retrieves the current record count in a specified block.
int HP_GetRecordCounter(int file_desc, int blockId);

// Returns the identifier of the last block in the heap file.
int HP_GetIdOfLastBlock(int file_desc);

// Retrieves the number of records that can fit in a block of the heap file.
int HP_GetMaxRecordsInBlock(int file_desc);

// Prints all entries(records) contained in the specified block of the heap file.
int HP_PrintBlockEntries(int file_desc, int blockId);



#endif // HP_FILE_H
