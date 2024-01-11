#ifndef MERGE_H
#define MERGE_H
#define RECORDS_NUM 10000

#include "sort.h"
#include "chunk.h"

/* Merges every b chunks of size chunkSize from the input file to the specified output file. The function takes input file and output file descriptors, chunk size, and the number of chunks to merge. It should internally use a CHUNK_Iterator and a CHUNK_RecordIterator.*/
void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc, int* filecounter);

int createAndPopulateHeapFile(char* filename);

#endif  // MY_HEADER_H