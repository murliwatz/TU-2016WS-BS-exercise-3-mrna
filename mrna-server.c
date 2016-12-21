/**
 * @file mrna-server.c
 * @brief main c file for the implementation of mrna-server
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
#include "mrna-server.h"

/** struct for shared memory */
static struct sm_data {
    bool c_set; /* 1 - client has set the data, 0 - server has set the data */
    int cur; /* cursor in dna */
    char data[4096];
};

/** name of this program */
static char* progname; 

int main(int argc, char** argv) {

	parse_args(argc, argv);

    key_t key = 1525669;
    int shmid;

    if ((shmid = shmget (key, sizeof(struct sm_data), IPC_CREAT | 0660)) == -1) {
        bail_out(EXIT_FAILURE, "create shared memory");
    }

    struct sm_data *data;

    if((data = (struct sm_data*)shmat(shmid, 0, 0)) == NULL) {
        bail_out(EXIT_FAILURE, "attatch shared memory");
    }

    data->c_set = 0;
    data->cur = 0;

    while(true) {
        if(data->c_set) {
            fprintf(stderr, "client has set data");
            data->c_set = false;
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
				bail_out(EXIT_FAILURE, "USAGE: mrna-server");		
		}
	}
}

/**
 *  @brief Free allocated resources
 */
static void free_resources(void) {

}