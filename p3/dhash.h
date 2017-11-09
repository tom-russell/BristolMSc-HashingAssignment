#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "neillsdl2.h"

/* Error Print Statements */
#define ERR_NO_FILE      "ERROR - 1 filename need to be passed to the program.\n"
#define ERR_FOPEN_FAIL   "ERROR - Failed to open the specified file.\n"
#define ERR_FCLOSE_FAIL  "ERROR - Failed to close the specified file.\n"
#define ERR_TABLE_FULL   "ERROR - Hash table has not been resized correctly.\n"
#define ERR_WORD_MISSING "ERROR - A word was not found in the hash table.\n"
#define ERR_EMPTY_FILE   "ERROR - No words were found in the test search file.\n"
#define ERR_LONG_STR     "ERROR - A word in the dictionary > MAXWORDLEN.\n"

/* SDL Parameters */
#define COLOURMAX 255
#define DELAYMS 1
#define W_BUFFER 25
#define PROGBARMAX 750
#define ELEMENT_H 425
#define BAROUTLINE_W 752
#define BAROUTLINE_H 102
#define BAROUTLINE_X 24
#define BAROUTLINE_Y 474
#define PROGBAR_H 100
#define PROGBAR_Y 475

typedef struct HashTableData {
	char **hash_table;
	int table_size;
	int max_table_load;
} HData;

typedef struct SDLDataStruct {
    double cells_per_pix;
    double col_increment;
    double *d_array;
	int prog_bar_val;
	int array_loc;
    int hash;
    
    SDL_Rect pixel_line;
    SDL_Rect clear_screen;
    SDL_Rect bar_outline;
    SDL_Rect prog_bar;
} SDLData;

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

/* Hashing Functions */
void InitialiseHashData(HData *hdata, int size);
void CreateHashTable(HData *hdata, char *filename, SDL_Simplewin *sw);
void LoadNextWord(char *curr_word, FILE *txt_file);
int AddToHashTable(HData *hdata, char *curr_word);
unsigned int HashFunc1(char *str);
unsigned int HashFunc2(char *str);
void ResizeHashTable(HData *hdata, SDL_Simplewin *sw, SDLData *sdl_data);
int PrimeReturn(int test);
void FreeHashTable(char **hash_table, int table_size);
/* SDL Functions */
void ResetDisplay(SDL_Simplewin *sw, SDLData *sdl_data);
void UpdateSDL(SDL_Simplewin *sw, SDLData *sdl_data, HData *hdata, int count);
void EndSDL(SDL_Simplewin *sw, SDLData *sdl_data); 
void InitialiseSDL(SDLData *sdl_data, int table_size);
