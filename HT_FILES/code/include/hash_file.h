#ifndef HASH_FILE_H
#define HASH_FILE_H

#include "bf.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


typedef enum HT_ErrorCode {
  HT_OK,
  HT_ERROR
} HT_ErrorCode;

typedef struct Record {
	int id;
	char name[15];
	char surname[20];
	char city[20];
} Record;

typedef struct Index_info{
  int global_depth;
  int file_desc;
  int size;
  int bucket_num;
  int max_rec;
  int last_block_id;
  int* block_ids;
} Index_info;

typedef struct Bucket_info{
  int local_depth;
  int rec_num;
} Bucket_info;



HT_ErrorCode HashStatistics(char* filename);
/*
 * Η συνάρτηση HT_Init χρησιμοποιείται για την αρχικοποίηση κάποιον δομών που μπορεί να χρειαστείτε. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_Init();

/*
 * Η συνάρτηση HT_CreateIndex χρησιμοποιείται για τη δημιουργία και κατάλληλη αρχικοποίηση ενός άδειου αρχείου κατακερματισμού με όνομα fileName. 
 * Στην περίπτωση που το αρχείο υπάρχει ήδη, τότε επιστρέφεται ένας κωδικός λάθους. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς επιστρέφεται HΤ_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_CreateIndex(
	const char *fileName,		/* όνομααρχείου */
	int depth
	);


/*
 * Η ρουτίνα αυτή ανοίγει το αρχείο με όνομα fileName. 
 * Εάν το αρχείο ανοιχτεί κανονικά, η ρουτίνα επιστρέφει HT_OK, ενώ σε διαφορετική περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_OpenIndex(
	const char *fileName, 		/* όνομα αρχείου */
  	int *indexDesc            /* θέση στον πίνακα με τα ανοιχτά αρχεία  που επιστρέφεται */
	);

/*
 * Η ρουτίνα αυτή κλείνει το αρχείο του οποίου οι πληροφορίες βρίσκονται στην θέση indexDesc του πίνακα ανοιχτών αρχείων.
 * Επίσης σβήνει την καταχώρηση που αντιστοιχεί στο αρχείο αυτό στον πίνακα ανοιχτών αρχείων. 
 * Η συνάρτηση επιστρέφει ΗΤ_OK εάν το αρχείο κλείσει επιτυχώς, ενώ σε διαφορετική σε περίπτωση κωδικός λάθους.
 */
HT_ErrorCode HT_CloseFile(
	int indexDesc 		/* θέση στον πίνακα με τα ανοιχτά αρχεία */
	);

/*
 * Η συνάρτηση HT_InsertEntry χρησιμοποιείται για την εισαγωγή μίας εγγραφής στο αρχείο κατακερματισμού. 
 * Οι πληροφορίες που αφορούν το αρχείο βρίσκονται στον πίνακα ανοιχτών αρχείων, ενώ η εγγραφή προς εισαγωγή προσδιορίζεται από τη δομή record. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κάποιος κωδικός λάθους.
 */
HT_ErrorCode HT_InsertEntry(
	int indexDesc,	/* θέση στον πίνακα με τα ανοιχτά αρχεία */
	Record record		/* δομή που προσδιορίζει την εγγραφή */
	);

/*
 * Η συνάρτηση HΤ_PrintAllEntries χρησιμοποιείται για την εκτύπωση όλων των εγγραφών που το record.id έχει τιμή id. 
 * Αν το id είναι NULL τότε θα εκτυπώνει όλες τις εγγραφές του αρχείου κατακερματισμού. 
 * Σε περίπτωση που εκτελεστεί επιτυχώς επιστρέφεται HP_OK, ενώ σε διαφορετική περίπτωση κάποιος κωδικός λάθους.
 */
HT_ErrorCode HT_PrintAllEntries(
	int indexDesc,	/* θέση στον πίνακα με τα ανοιχτά αρχεία */
	int *id 				/* τιμή του πεδίου κλειδιού προς αναζήτηση */
	);



///////////////////////////////////////////// -- EXTRA FUNCTIONS -- ///////////////////////////////////////////// 

//https://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
unsigned int hash_func(unsigned int	key);

//Transforms an integer to binary (string) 
void intToBinary(int key, char* string_key);


//Returns the (gdepth) lsb in string_key variable
void least_significant_bits(char* msb, char* string_key, int gdepth);


//Transforms string to integer
int stringToInt(char* msb);


//Calls all the needed fucntions to return the final position of the hashed key
int final_key_index(int id, int bit_num);


//Prints all records of a file
void printallrecs(Index_info* index);




#endif // HASH_FILE_H
