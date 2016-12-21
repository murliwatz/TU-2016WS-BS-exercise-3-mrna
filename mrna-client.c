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
    int pos_start;
    int pos_end;
    char payload[4096];
} sm_data;

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

    struct sm_data *data = &sm_data;

    if((data = shmat(shmid, 0, 0)) == NULL) {
        bail_out(EXIT_FAILURE, "attatch shared memory");
    }

	char buffer[4096];
	int pos[2];
	pos[0] = 0;
	pos[1] = 0;

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

		}

		if(arg == 'n') { 

			memcpy(data->payload, buffer, 4096);
			data->pos_start = pos[0];
    		data->pos_end = pos[1];
			data->c_set = 1;

			while(data->c_set);

			pos[0] = data->pos_end;
			pos[1] = data->pos_end;

			if(data->pos_start == data->pos_end) {
				fprintf(stdout, "End reached [%d/%lu], send r to reset.\n", data->pos_start, strlen(buffer));
			} else {
				fprintf(stdout, "Protein sequence found [%d/%lu] to [%d/%lu]: %s \n", data->pos_start, strlen(buffer), data->pos_end, strlen(buffer), data->payload);
			}
		
		}

		if(arg == 'r') {
			pos[0] = 0;
			pos[1] = 0;
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
