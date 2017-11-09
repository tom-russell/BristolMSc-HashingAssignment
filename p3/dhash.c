#include "dhash.h"

#define MAXWORDLEN 15
#define MAXLOADFRACTION 0.6
#define SIZEINCREASE 2
#define PRIME 31

void InitialiseHashData(HData *hdata, int size)
{
    int prime_size = PrimeReturn(size);

    hdata->hash_table = (char **)calloc(prime_size, sizeof(char *));
    hdata->table_size = prime_size;
    hdata->max_table_load = (int)(prime_size * MAXLOADFRACTION);
}

void CreateHashTable(HData *hdata, char *filename, SDL_Simplewin *sw)
{
    int count = 0;
    char curr_word[MAXWORDLEN];
    
    FILE *dict_file;
    SDLData sdl_data;
    
    InitialiseSDL(&sdl_data, hdata->table_size);
    ResetDisplay(sw, &sdl_data);
    
    /* Open dictionary file, exit if fopen fails */
    dict_file = fopen(filename, "r");
    if (dict_file == NULL) {
        fprintf(stderr, ERR_FOPEN_FAIL);
        exit(fopen_fail);
    }

    /* Load word & add it to the hash table, repeat for all words */
    LoadNextWord(curr_word, dict_file);
    while (curr_word[0] != '\0') {
        count++;
        sdl_data.hash = AddToHashTable(hdata, curr_word);
        
        /* Update the SDL window each time a word is hashed */
        UpdateSDL(sw, &sdl_data, hdata, count);
        SDL_Delay(DELAYMS);

        /* If the hash table is too full, rebuild with 2x size */
        if (count > hdata->max_table_load) {
            ResetDisplay(sw, &sdl_data);
            ResizeHashTable(hdata, sw, &sdl_data);
        }
        LoadNextWord(curr_word, dict_file);
    }
    
    if (fclose(dict_file) != 0) {
        fprintf(stderr, ERR_FCLOSE_FAIL);
        exit(fclose_fail);
    }
    EndSDL(sw, &sdl_data);
}

void InitialiseSDL(SDLData *sdl_data, int table_size) 
{
    /* Initalise SDL drawing variables */
    sdl_data->cells_per_pix = (int)ceil((double)table_size / WWIDTH);
    sdl_data->col_increment = COLOURMAX / sdl_data->cells_per_pix;
    sdl_data->d_array = calloc(WWIDTH, sizeof(double));
    
    /* Initialse SDL object data */
    sdl_data->clear_screen.w = WWIDTH;
    sdl_data->clear_screen.h = WHEIGHT;
	sdl_data->clear_screen.x = 0;
    sdl_data->clear_screen.y = 0;
    sdl_data->pixel_line.w = 1;
    sdl_data->pixel_line.h = ELEMENT_H;
    sdl_data->pixel_line.y = W_BUFFER;
    sdl_data->bar_outline.w = BAROUTLINE_W;
    sdl_data->bar_outline.h = BAROUTLINE_H;
    sdl_data->bar_outline.x = BAROUTLINE_X;
    sdl_data->bar_outline.y = BAROUTLINE_Y;
    sdl_data->prog_bar.h = PROGBAR_H;
    sdl_data->prog_bar.x = W_BUFFER;
    sdl_data->prog_bar.y = PROGBAR_Y;
}

void ResetDisplay(SDL_Simplewin *sw, SDLData *sdl_data) 
{
    int i;
    
    /* Reset the display to black, and draw the progress bar outline */
    Neill_SDL_SetDrawColour(sw, 0, 0, 0);
    SDL_RenderFillRect(sw->renderer, &sdl_data->clear_screen);
    Neill_SDL_SetDrawColour(sw, COLOURMAX, COLOURMAX, COLOURMAX);
    SDL_RenderDrawRect(sw->renderer, &sdl_data->bar_outline);
    
    for (i = 0; i < WWIDTH; i++) {
        sdl_data->d_array[i] = 0.0;
    }
}

void UpdateSDL(SDL_Simplewin *sw, SDLData *sdl_data, HData *hdata, int count)
{
    /* Update sdl data for the newly added hash */ 
	sdl_data->array_loc =\
        (int)(((double)sdl_data->hash / hdata->table_size) * WWIDTH);
    sdl_data->d_array[sdl_data->array_loc] += sdl_data->col_increment;
    sdl_data->prog_bar_val = (double)count/(hdata->max_table_load)*PROGBARMAX;
    sdl_data->pixel_line.x = sdl_data->array_loc;
    
    /* Update the colour of the relevant hash table cell */
    Neill_SDL_SetDrawColour(sw, sdl_data->d_array[sdl_data->array_loc], 0, 0);
    SDL_RenderFillRect(sw->renderer, &sdl_data->pixel_line);
    
    /* Update the progress bar */
    Neill_SDL_SetDrawColour(sw, 0, 0, COLOURMAX);
    sdl_data->prog_bar.w = sdl_data->prog_bar_val;
    SDL_RenderFillRect(sw->renderer, &sdl_data->prog_bar);
    
    /* Render the updated graphics in the SDL window */
    SDL_RenderPresent(sw->renderer);
    SDL_UpdateWindowSurface(sw->win); 
    Neill_SDL_Events(sw);
    
    /* Exit if user has attempted to quit program */
    if (sw->finished) {
        atexit(SDL_Quit);
        exit(0);
    }
 }

/* Turn the progress bar green to indicate hashing has finished */
void EndSDL(SDL_Simplewin *sw, SDLData *sdl_data) 
{
    /* Draw progress bar in green if hashing has completed */
    Neill_SDL_SetDrawColour(sw, 0, COLOURMAX, 0);
    sdl_data->prog_bar.w = sdl_data->prog_bar_val;
    SDL_RenderFillRect(sw->renderer, &sdl_data->prog_bar);
    
    /* Update the SDL window for the final time */
    SDL_RenderPresent(sw->renderer);
    SDL_UpdateWindowSurface(sw->win); 
    Neill_SDL_Events(sw);
    
    /* Exit if user has attempted to quit program */
    if (sw->finished) {
        atexit(SDL_Quit);
        exit(0);
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
int AddToHashTable(HData *hdata, char *curr_word)
{
    int hash1, hash2, hash_t;

    /* Calculate the hashes for the current word */
    hash1 = HashFunc1(curr_word) % hdata->table_size;
    hash2 = (HashFunc2(curr_word) % (hdata->table_size - 1)) + 1;
    hash_t = hash1;
    
    /* Loop taking hash2 away from hash_t until an empty space is found */
    do {
        /* If the location hash1 is free in the hash_table, add the word */
        if (hdata->hash_table[hash_t] == NULL) {
            hdata->hash_table[hash_t] =\
                calloc(strlen(curr_word) + 1, sizeof(char));
            strcpy(hdata->hash_table[hash_t], curr_word);

            return hash_t;
        }
        
        hash_t -= hash2;
        /* If the hash goes below 0, wrap back past the end of the array */
        if (hash_t < 0) {
            hash_t += hdata->table_size;
        }
    }
    while (hash_t != hash1);
    
    fprintf(stderr, ERR_TABLE_FULL);
    exit(hash_table_full);
}

/* Calculates a hash using start & end 2 chars and word length */
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

/* Calculates a hash using each char of the word & word length */
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
void ResizeHashTable(HData *hdata, SDL_Simplewin *sw, SDLData *sdl_data)
{
    char **old_hash_table = hdata->hash_table;
    int old_table_size = hdata->table_size;
    int i, count = 0;
    
    /* Create the new bigger hash table, currently empty */
    InitialiseHashData(hdata, PrimeReturn(hdata->table_size * SIZEINCREASE));
    
    /* Update the SDL variables based on the new table size */
	sdl_data->cells_per_pix = (int)ceil((double)hdata->table_size / WWIDTH);
	sdl_data->col_increment = 255 / sdl_data->cells_per_pix;

    for (i = 0; i < old_table_size; i++) {
        /* For each word in the old hash table, add to the new table */
        if (old_hash_table[i] != NULL) {
            count++;
            
            /* Add the new word to the hash table and update the SDL window */
            sdl_data->hash = AddToHashTable(hdata, old_hash_table[i]);
            UpdateSDL(sw, sdl_data, hdata, count);
			SDL_Delay(DELAYMS);
        }
    }
    FreeHashTable(old_hash_table, old_table_size);
}

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
