### File DBMS Algorithms For Sorting - Searching Using C

TEAM MEMBERS: 

Michaela Koulloli 

Andreas Pavlou 

 ### 1) Heap - File Algorithm implementation for searching

Tests for the proper functioning of the code were conducted based on the provided main, gradually adapting it, so we did not create our own main.

We modified the Makefile so that each time the data.db file is deleted, it compiles and runs using the command:
- ```c
  make hp run 
  
Changes were made to run valgrind for memory leaks with the:
- ```c
  make hp valgrind
  
Some remarks regarding the code:
- The struct HP_BLOCK_INFO, which we were asked to implement, is implemented in the hp_file.c file.

- Additionally, we chose to store the block_info of each block at the beginning of each block instead of at the end.

- In main, function, we do not use BF_CLOSE() because we do it in HP_CLOSEFILE.

- We did not use memcpy(); we did it manually.



 ### 2) Hash Table - File Algorithm implementation for searching 

 
Tests for the proper functioning of the code were conducted based on the provided main, gradually adapting it, so we did not create our own main.

Makefile: We modified the Makefile so that each time the data.db is deleted and added headers, so changes were made to it as well, and it compiles and runs using the command
- ```c
     make ht run

Remarks regarding the code:

Organization: Declarations of the extra functions we created and the structs are in include/hash_file.h, while their implementations are in hash_functions/functions.c.
Design: We chose to initialize the table where we keep the open files as global (Index_info* open_files[MAX_OPEN_FILES]), in which we store a pointer to the metadata of the file (struct Index_info) at position i, corresponding to the indexDesc.
File Metadata: Stored in the first block of each file and contain both the Hash Table (int* block_ids) and data related to the file. Block/Bucket Metadata: (struct bucket_info) Stored at the beginning of each block and contain data related to each block/bucket.
HT_Init(): We use it to initialize the Index_info structure that holds the file's metadata, so we added an argument (indexdesc) that was not in the initial prototype provided to us. We call it in HT_CreateIndex(), so we don't call it in the main.
HT_InsertEntry(): Acknowledgment to the insert: The main change made to the insert is that entries are hashed based on the least significant bits instead of the most significant bits. This results in differentiation:

- (1) In doubling.
- (2) In the split, when the specific bucket that needs to be split "points" to more than two pointers.

Explained:
- (1) During doubling, the new array in the first half, the "old" one, remains the same, and the new positions point to the corresponding pre-existing buckets, without considering the most significant bit.


                            Old HT:         New HT:

       	                                    00 --> 1

                            0 --> 1         01 --> 2

                            1 --> 2         10 --> 1

                                            11 --> 2


                          Where 1, 2 are the ids of buckets(Blocks).


  (2) In this case, the split occurs by creating a new bucket, to which the original pointer that underwent hashing and its buddy will point. To locate the buddy, which has a different most significant bit, a new bucket is created.


## 3) Merge Sort Algorithm implementation for searching (Was Only Allowed to Use Chatgpt)
Chats with GPT:

- shouldSwap / CHUNK_CreateIterator: https://chat.openai.com/share/472bf9ad-39ec-4217-acc1-672165ad4390
- Chunk Iterator Creation: https://chat.openai.com/share/fdd48c25-458c-420a-b23e-b2e086c1aaa4
- sort_FileInChunks: https://chat.openai.com/share/2caab5e1-df97-4318-aa03-32b6554c4ad7
- Merge Sort Function (C): https://chat.openai.com/share/d12e9b3b-7423-42ee-9927-646f258bb5f7


Correctness Testing:
The correctness of the program has been tested for various bWay and chunkSize values, for records 10,000+, without encountering any issues, successfully sorting the file.

Implementation Notes:
There were no major variations in the implementation of the assignment as it was done based on the presentation provided in class. Our conversations with ChatGPT, as seen in the links above, helped us make minor adjustments to adapt the code to our data. Minimal to no changes were made during the implementation of ShouldSwap/Chunks/Sort. Regarding the merge function, several attempts were made until we could be understood, as ChatGPT seems unable to grasp concepts like blocks/chunks, etc., and adjust the code accordingly. Therefore, we chose to refer to structures like arrays, which are more familiar, to make ourselves clearer. After our last attempt, shown in the link above, we managed to achieve better results by making some changes to make the code cleaner. In the conversion to block/file levels, we used functions from the library provided to us.

Program Verification:

10-Way External Sort

MAX_RECORDS = 10,000
bWay = 10
Blocks/Chunk = 5
Intermediate Files Created/Iterations = 4
2-Way External Sort

MAX_RECORDS = 10,000
bWay = 2
Blocks/Chunk = 5
Intermediate Files Created/Iterations = 9
As shown above, for 10,000 records, the utility of multi-way sorting is already evident. For 10-way, 4 intermediate files are created, while for 2-way (classic pairwise merge), 9 intermediate files are created, as expected. This way, space and time are saved.

You can try the test for 100,000 Records!






