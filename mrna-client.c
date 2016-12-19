/**
 * @file websh.c
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
#include "mrna-client.h"

/** name of this program */
static char* progname; 

int main(int argc, char** argv) {

	parse_args(argc, argv);

	print_commands();

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