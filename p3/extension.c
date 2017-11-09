#include "dhash.h"

#define STARTSIZE 1000

int main(int argc, char **argv)
{
	HData hdata;
	SDL_Simplewin sw;
	InitialiseHashData(&hdata, STARTSIZE);

	Neill_SDL_Init(&sw);	

	/* Exit if not passed a dictionary */
	if (argc != 2) {
		fprintf(stderr, ERR_NO_FILE);
		exit(no_file_passed);
	}

	/* Set up the hash table for the given dictionary file */
	CreateHashTable(&hdata, argv[1], &sw);
    
    /* Free up all dynamically allocated space used in the hash table */
	FreeHashTable(hdata.hash_table, hdata.table_size);
    
	/* Wait until the user presses esc or closes the SDL window */
	while(!sw.finished) {
		Neill_SDL_Events(&sw);
	}

	/* Clear up graphics subsystems */
   	atexit(SDL_Quit);

	return 0;
}