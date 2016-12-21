/**
 * @file mrna-client.c
 * @brief main c file for the implementation of mrna-client
 * @author Paul Pröll, 1525669
 * @date 2016-11-15
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include "mrna-client.h"

/** struct for shared memory */
static struct sm_data {
    bool c_set; /* 1 - client has set the data, 0 - server has set the data */
    int cur; /* cursor in dna */
    char data[4096];
};

/** name of this program */
static char* progname;

int main(int argc, char** argv) {

	char arg;

	parse_args(argc, argv);

	print_commands();

	key_t key = 1525669;
    int shmid;

    if ((shmid = shmget (key, sizeof(struct sm_data), 0)) == -1) {
        bail_out(EXIT_FAILURE, "create shared memory");
    }

    struct sm_data *data;

    if((data = shmat(shmid, 0, 0)) == NULL) {
        bail_out(EXIT_FAILURE, "attatch shared memory");
    }

    data->c_set = 1;
    data->cur = 0;

	char buffer[4096];

	while(arg != 'q') {

		fprintf(stdout, "Enter new command. (Enter to end input): ");

		if((arg = fgetc(stdin)) == EOF) {
			bail_out(EXIT_FAILURE, "read argument");
		}

		if(fgetc(stdin) != '\n') {
			fprintf(stderr, "%s\n", "Error: Invalid argument");
			while(fgetc(stdin) != '\n');
			continue;
		}

		if(arg != 's' && arg != 'n' && arg != 'r' && arg != 'q') {
			fprintf(stderr, "%s\n", "Error: Invalid argument");
			continue;
		}

		if(arg == 's') {
			
			fprintf(stdout, "Enter new mRNA sequence. (Newline to end input): ");

			if(fgets(buffer, 4096, stdin) == NULL) {
				bail_out(EXIT_FAILURE, "read line");
			}

			for(int i = 0; buffer[i] != '\0'; i++) {
				if(buffer[i] != 'A' && buffer[i] != 'C' && buffer[i] != 'G' && buffer[i] != 'U') {
					memmove(&buffer[i], &buffer[i + 1], strlen(buffer) - i);
					i = 0; 
				}
			}

			if(strlen(buffer) % 3 != 0) {
				bail_out(EXIT_FAILURE, "wrong dna length");
			}

			fprintf(stderr, "%s\n", buffer);

		}

		if(arg == 'n') { 

			char *mrna = NULL;
			int borders[2];
			mrna = get_mrna(buffer, borders);

			fprintf(stdout, "Protein sequence found [%d/%lu] to [%d/%lu]: %s \n", borders[0], strlen(buffer), borders[1], strlen(buffer), mrna);
		
		}

	}

	exit(EXIT_SUCCESS);	
}

/**
 * @brief Prints fmt on stderr, free resources and closes program with exitcode
 * @param exitcode Exitcode that should be returned for termination of process
 * @param fmt String to print out
 * @param ...
 */
static void bail_out(int exitcode, const char *fmt, ...)
{
    va_list ap;

    (void) fprintf(stderr, "%s: ", progname);
    if (fmt != NULL) {
        va_start(ap, fmt);
        (void) vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    if (errno != 0) {
        (void) fprintf(stderr, ": %s", strerror(errno));
    }
    (void) fprintf(stderr, "\n");

    free_resources();
    exit(exitcode);
}

/**
 * @brief Parses the program arguments
 * @param argc Count of arguments
 * @param argv Array of arguments
 */
static void parse_args(int argc, char** argv) {
	int opt;

	if(argc > 0) {
		progname = argv[0];
	}
	while((opt = getopt(argc, argv, "")) != -1) {
		switch(opt) {
			default:
				bail_out(EXIT_FAILURE, "USAGE: mrna-client");		
		}
	}
}

/**
 *  @brief Free allocated resources
 */
static void free_resources(void) {
	
}

/*
 * @brief prints available commands on stdout
 */
static void print_commands(void) {
	fprintf(stdout, "%s\n", "Available commands:");
	fprintf(stdout, "%s\n", " s - submit a new mRNA sequence");
	fprintf(stdout, "%s\n", " n - show next protein sequence in active mRNA sequence");
	fprintf(stdout, "%s\n", " r - reset active mRNA sequence");
	fprintf(stdout, "%s\n", " q - close this client");
}

/*
 * @brief Returns a codon by bases
 */
static char get_codon(char *bases) {
	for(int i = 0; i < 3; i++) {
		if(bases[i] != 'A' && bases[i] != 'C' && bases[i] != 'G' && bases[i] != 'U') {
			bail_out(EXIT_FAILURE, "Not a correct base");
		}
	}

	char codon = 0;

	switch(bases[0]) {
		case 'U':
			if(bases[1] == 'U') {
				if(bases[2] == 'U' || bases[2] == 'C') {
					codon = 'F';
				}
				if(bases[2] == 'A' || bases[2] == 'G') {
					codon = 'L';
				}
			}
			if(bases[1] == 'C') {
				codon = 'S';
			}
			if(bases[1] == 'A') {
				if(bases[2] == 'U' || bases[2] == 'C') {
					codon = 'Y';
				}
				if(bases[2] == 'A' || bases[2] == 'G') {
					codon = 0;
				}
			}
			if(bases[1] == 'G') {
				if(bases[2] == 'U' || bases[2] == 'C') {
					codon = 'C';
				}
				if(bases[2] == 'G') {
					codon = 'W';
				}
			}
			break;
		case 'C':
			if(bases[1] == 'U') {
				codon = 'L';
			}
			if(bases[1] == 'C') {
				codon = 'P';
			}
			if(bases[1] == 'A') {
				if(bases[2] == 'U' || bases[2] == 'C') {
					codon = 'H';
				}
				if(bases[2] == 'A' || bases[2] == 'G') {
					codon = 'Q';
				}
			}
			if(bases[1] == 'G') {
				codon = 'R';
			}
			break;
		case 'A':
			if(bases[1] == 'U') {
				if(bases[2] == 'U' || bases[2] == 'C' || bases[2] == 'A') {
					codon = 'I';
				}
				if(bases[2] == 'G') {
					codon = 'M';
				}
			}
			if(bases[1] == 'C') {
				codon = 'T';
			}
			if(bases[1] == 'A') {
				if(bases[2] == 'U' || bases[2] == 'C') {
					codon = 'N';
				}
				if(bases[2] == 'A' || bases[2] == 'G') {
					codon = 'K';
				}
			}
			if(bases[1] == 'G') {
				if(bases[2] == 'U' || bases[2] == 'C') {
					codon = 'S';
				}
				if(bases[2] == 'A' || bases[2] == 'G') {
					codon = 'R';
				}
			}
			break;
		case 'G':
			if(bases[1] == 'U') {
				codon = 'V';
			}
			if(bases[1] == 'C') {
				codon = 'A';
			}
			if(bases[1] == 'A') {
				if(bases[2] == 'U' || bases[2] == 'C') {
					codon = 'D';
				}
				if(bases[2] == 'A' || bases[2] == 'G') {
					codon = 'E';
				}
			}
			if(bases[1] == 'G') {
				codon = 'G';
			}
			break;
	}

	return codon;
}

/*
 * @brief Returns a mRNA
 */
static char *get_mrna(char* dna, int* borders) {
	char* mrna_new = NULL;

	bool started = false;

	int mrna_count = 0;

	int i = 0;
	while(i < strlen(dna)) {
		char codon = get_codon(dna + i);
		if(codon == 'M') {
			started = true;
			borders[0] = i + 3;
		} else if(codon == '\0') {
			if(started) {
				borders[1] = i;
				break;
			}
		} else {
			if(started) {
				if(mrna_new == NULL) {
					if((mrna_new = malloc(1)) == NULL) {
						bail_out(EXIT_FAILURE, "allocation memory");
					}
				} else {
					if((mrna_new = realloc(mrna_new, (mrna_count + 1) + 1)) == NULL) {
						bail_out(EXIT_FAILURE, "allocation memory");
					}
				}
				mrna_new[mrna_count++] = codon;
			}
		}
		if(started) {
			i += 3;
		} else {
			i++;
		}
	}

	return mrna_new;
}