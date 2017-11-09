#include "dhash.h"
#define STARTSIZE 1000

int main(int argc, char **argv)
{
    HashData hashdata;
    InitHashData(&hashdata, STARTSIZE);

    /* Exit if not passed a dictionary and word test file */
    if (argc != 3) {
        fprintf(stderr, ERR_NO_FILE);
        exit(no_file_passed);
    }

    /* Set up the hash table for the given dictionary file */
    CreateHashTable(&hashdata, argv[1]);

    /* Search for the test words in the hash table */
    printf("Table size = %d. ", hashdata.table_size);
    printf("The words took an average of %f lookups to find.\n",\
            HashSearchTest(&hashdata, argv[2])
    );
    
    /* Free up all dynamically allocated space using in the hash table */
    FreeHashTable(hashdata.hash_table, hashdata.table_size);

    return 0;
}