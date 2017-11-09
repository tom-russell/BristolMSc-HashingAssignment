#include "shash.h"

#define PRIME 31

void InitialiseHashData(HashData *hashdata, int size)
{
    int prime_size = PrimeReturn(size);

    hashdata->hash_table = (HashElem **)calloc(prime_size, sizeof(HashElem *));
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

    /* Load in next word & add it to the hash table, repeat for all words */
    LoadNextWord(curr_word, dict_file);
    while (curr_word[0] != '\0') {
        AddToHashTable(hashdata, curr_word);
        count++;

        /* If the hash table is too full, rebuild the table 2x size */
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
    int hash = HashFunc(curr_word) % hashdata->table_size;
    HashElem *new_element;
    HashElem *prev_pointer;
    HashElem *temp_pointer = hashdata->hash_table[hash];

    /* Initialise the new element with the current word */
    new_element = calloc(1, sizeof(HashElem));
    new_element->word = calloc(strlen(curr_word) + 1, sizeof(char));
    strcpy(new_element->word, curr_word);

    /* Point the hashtable location at the newly created element */
    if (hashdata->hash_table[hash] == NULL) {
        hashdata->hash_table[hash] = new_element;
    }
    /* If the location is taken, follow the list to find a free space */
    else {
        do {
            prev_pointer = temp_pointer;
            temp_pointer = temp_pointer->next;
        }
        while (temp_pointer != NULL);
        prev_pointer->next = new_element;
    }
}

/* Calculates a hash using each char & string length */
unsigned int HashFunc(char *str)
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
    HashElem **old_hash_table = hashdata->hash_table;
    HashElem *temp_pointer;
    int i, old_table_size = hashdata->table_size;

    /* Create the new bigger hash table, currently empty */
    InitialiseHashData(hashdata, hashdata->table_size * SIZEINCREASE);

    for (i = 0; i < old_table_size; i++) {
        /* For each element in the old table, add to the new table & free it */
        temp_pointer = old_hash_table[i];
        while (temp_pointer != NULL) {
            AddToHashTable(hashdata, temp_pointer->word);
            temp_pointer = temp_pointer->next;
        }

    }
    FreeHashTable(old_hash_table, old_table_size);
}

/* For a given input integer, return the next highest prime number */
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
void FreeHashTable(HashElem **hash_table, int table_size)
{
    int i;
    HashElem *temp_pointer;
    HashElem *next_pointer;

    for (i = 0; i < table_size; i++) {
        /* If the hash location has a chain, free elements in the chain */
        if (hash_table[i] != NULL) {
            next_pointer = hash_table[i];

            do {
                temp_pointer = next_pointer;
                next_pointer = next_pointer->next;

                free(temp_pointer->word);
                free(temp_pointer);
            }
            while (next_pointer != NULL);
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

    /* Load word and search for it in the hashtable, repeat for all words */
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

    int hash, counter = 1;
    HashElem *temp_pointer;

    /* Calculate hash for the current word */
    hash = HashFunc(curr_word) % hashdata->table_size;

    /* If hash location is NULL, the word is not in the hash table */
    if (hashdata->hash_table[hash] == NULL) {
        fprintf(stderr, ERR_WORD_MISSING);
        exit(word_not_found);
    }
    /* If we have found the word return counter value */
    if (strcmp(hashdata->hash_table[hash]->word, curr_word) == 0) {
        return counter;
    }

    /* Follow along hash chain until word is found */
    temp_pointer = hashdata->hash_table[hash]->next;
    while (temp_pointer != NULL) {
        counter++;

        if (strcmp(temp_pointer->word, curr_word) == 0) {
            return counter;
        }
        temp_pointer = temp_pointer->next;
    }
    fprintf(stderr, ERR_WORD_MISSING);
    exit(word_not_found);
}
