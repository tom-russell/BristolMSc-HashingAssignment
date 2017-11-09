#include "dhash.h"

#define PRIME 31

void InitHashData(HashData *hashdata, int size)
{
    int prime_size = PrimeReturn(size);

    hashdata->hash_table = (char **)calloc(prime_size, sizeof(char *));
    hashdata->table_size = prime_size;
    hashdata->max_table_load = (int)(prime_size * MAXLOADFRACTION);
}

void CreateHashTable(HashData *hashdata, char *filename)
{
    int count = 0;
    char curr_word[MAXWORDLEN];

    FILE *dict_file;
    /* Open dictionary file, exit if fopen fails */
    dict_file = fopen(filename, "r");
    if (dict_file == NULL) {
        fprintf(stderr, ERR_FOPEN_FAIL);
        exit(fopen_fail);
    }

    /* Load in word & add it to the hash table, repeat for all words */
    LoadNextWord(curr_word, dict_file);
    while (curr_word[0] != '\0') {
        AddToHashTable(hashdata, curr_word);
        count++;

        /* If the hash table is too full, rebuild table with 2x size */
        if (count > hashdata->max_table_load) {
            ResizeHashTable(hashdata);
        }
        LoadNextWord(curr_word, dict_file);
    }

    if (fclose(dict_file) != 0) {
        fprintf(stderr, ERR_FCLOSE_FAIL);
        exit(fclose_fail);
    }
}

/* Copies the next word from file into the curr_word string */
void LoadNextWord(char *curr_word, FILE *txt_file)
{
    int counter = 0;
    char c;

    while ((c = getc(txt_file)) != '\n' && c != EOF) {
        if (isalpha(c) != 0) {
            curr_word[counter] = c;
            counter++;

            if (counter > MAXWORDLEN - 1) {
                fprintf(stderr, ERR_LONG_STR);
                exit(str_too_long);
            }
        }
    }
    curr_word[counter] = '\0';
}

/* Finds a hash for the current word, then places it in the hash table */
void AddToHashTable(HashData *hashdata, char *curr_word)
{
    int hash1, hash2, hash_t;

    /* Calculate the hashes for the current word */
    hash1 = HashFunc1(curr_word) % hashdata->table_size;
    hash2 = (HashFunc2(curr_word) % (hashdata->table_size - 1)) + 1;
    hash_t = hash1;

    /* Loop taking hash2 away from hash_t until an empty space is found */
    do {
        /* If the location hash1 is free in the hash_table, add the word */
        if (hashdata->hash_table[hash_t] == NULL) {
            hashdata->hash_table[hash_t] =\
                calloc(strlen(curr_word)+1, sizeof(char));
            strcpy(hashdata->hash_table[hash_t], curr_word);

            return;
        }

        hash_t -= hash2;
        /* If the hash goes below 0, wrap back past the end of the array */
        if (hash_t < 0) {
            hash_t += hashdata->table_size;
        }
    }
    while (hash_t != hash1);

    fprintf(stderr, ERR_TABLE_FULL);
    exit(hash_table_full);
}

/* Calculates a hash using the start & end 2 chars and string length */
unsigned int HashFunc1(char *str)
{
    unsigned int c1, c2, cn1, cn2;
    unsigned int hash;
    unsigned int n;

    n = (unsigned int)strlen(str);
    c1 = str[0];
    c2 = str[1];
    cn1 = str[n - 1];
    cn2 = str[n - 2];

    hash = (c1*c2 + cn1*cn2) * n;

    return hash;
}

/* Calculates a hash using each char & string length */
unsigned int HashFunc2(char *str)
{
    int i;
    int n = strlen(str);
    unsigned int hash = 0;

    for (i = 0; i < n; i++) {
        hash = str[i] + PRIME * hash;
    }

    return hash;
}

/* Creates a new larger hash table, moves the old values into the new table */
void ResizeHashTable(HashData *hashdata)
{
    char **old_hash_table = hashdata->hash_table;
    int i, old_table_size = hashdata->table_size;
    /* Create the new bigger hash table, currently empty */
    InitHashData(hashdata, PrimeReturn(hashdata->table_size * SIZEINCREASE));

    for (i = 0; i < old_table_size; i++) {
        /* For each word in the old hash table, add to the new table */
        if (old_hash_table[i] != NULL) {
            AddToHashTable(hashdata, old_hash_table[i]);
        }
    }
    FreeHashTable(old_hash_table, old_table_size);
}

/* From a given input integer, find the next highest prime number */
int PrimeReturn(int test)
{
    int i;

    for (i = 2; i < test / 2; i++) {
        if (test % i == 0) {
            i = 1;
            test++;
        }
    }

    return test;
}

/* Loops to first free each word in the table, the frees the hash table */
void FreeHashTable(char **hash_table, int table_size)
{
    int i;

    for (i = 0; i < table_size; i++) {
        if (hash_table[i] != NULL) {
            free(hash_table[i]);
        }
    }
    free(hash_table);
}

double HashSearchTest(HashData *hashdata, char *filename)
{
    char curr_word[MAXWORDLEN];
    int total_lookups = 0;
    int count = 0;

    FILE *test_file;
    /* Open test_file file, exit if fopen fails */
    test_file = fopen(filename, "r");
    if (test_file == NULL) {
        fprintf(stderr, ERR_FOPEN_FAIL);
        exit(fopen_fail);
    }

    /* Load in the next word and search for it in the hashtable, 
     * repeat for all words in file */
    LoadNextWord(curr_word, test_file);
    while (curr_word[0] != '\0') {
        total_lookups += WordSearch(hashdata, curr_word);
        count++;
        LoadNextWord(curr_word, test_file);
    }

    /* Exit if fclose fails */
    if (fclose(test_file) != 0) {
        fprintf(stderr, ERR_FCLOSE_FAIL);
        exit(fclose_fail);
    }

    /* Exit if no words were found in test_file */
    if (count == 0) {
        fprintf(stderr, ERR_EMPTY_FILE);
        exit(search_file_empty);
    }

    return (double) total_lookups / count;
}

int WordSearch(HashData *hashdata, char *curr_word) {

    int hash1, hash2, hash_t, counter = 1;

    /* Calculate hash1 for the current word */
    hash1 = HashFunc1(curr_word) % hashdata->table_size;
    hash2 = (HashFunc2(curr_word) % (hashdata->table_size - 1)) + 1;
    hash_t = hash1;

    do {
        /* If hasht location is NULL, the word is not in the hash table */
        if (hashdata->hash_table[hash_t] == NULL) {
            fprintf(stderr, ERR_WORD_MISSING);
            exit(word_not_found);
        }

        /* If we have found the word return counter value */
        if (strcmp(hashdata->hash_table[hash_t], curr_word) == 0) {
            return counter;
        }

        hash_t -=hash2;
        /* If  hash_t goes below 0, wrap back past the end of the array */
        if (hash_t < 0) {
            hash_t += hashdata->table_size;
        }
        counter++;
    }
    while (hash_t != hash1);

    fprintf(stderr, ERR_TABLE_FULL);
    exit(hash_table_full);
}