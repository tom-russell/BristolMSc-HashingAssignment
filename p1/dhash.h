#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXWORDLEN 15
#define MAXLOADFRACTION 0.6
#define SIZEINCREASE 2

/* Error Print Statements */
#define ERR_NO_FILE      "ERROR - 2 filenames need to be passed to the program.\n"
#define ERR_FOPEN_FAIL   "ERROR - Failed to open the specified file.\n"
#define ERR_FCLOSE_FAIL  "ERROR - Failed to close the specified file.\n"
#define ERR_TABLE_FULL   "ERROR - Hash table has not been resized correctly.\n"
#define ERR_WORD_MISSING "ERROR - A word was not found in the hash table.\n"
#define ERR_EMPTY_FILE   "ERROR - No words were found in the test search file.\n"
#define ERR_LONG_STR     "ERROR - A word in the dictionary is > MAXWORDLEN.\n"

typedef struct HashTableData {
    char **hash_table;
    int table_size;
    int max_table_load;
} HashData;

enum Exit_Codes {
    no_file_passed = 5,
    fopen_fail = 6,
    fclose_fail = 7,
    hash_table_full = 8,
    word_not_found = 9,
    search_file_empty = 10,
    str_too_long = 11
};

enum Boolean {
    false,
    true
};

void InitHashData(HashData *hashdata, int size);
void CreateHashTable(HashData *hashdata, char *filename);
void LoadNextWord(char *curr_word, FILE *txt_file);
void AddToHashTable(HashData *hashdata, char *curr_word);
unsigned int HashFunc1(char *str);
unsigned int HashFunc2(char *str);
void ResizeHashTable(HashData *hashdata);
int PrimeReturn(int test);
void FreeHashTable(char **hash_table, int table_size);
double HashSearchTest(HashData *hashdata, char *filename);
int WordSearch(HashData *hashdata, char *curr_word);