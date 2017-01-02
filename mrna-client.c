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
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include "mrna-client.h"

int main(int argc, char** argv) {

	char arg;

	parse_args(argc, argv);

	act.sa_handler = signal_handler;
    if(sigemptyset (&act.sa_mask) < 0) {
        bail_out(EXIT_FAILURE, "set sig'mask empty");
    }
    act.sa_flags = 0;

    if(sigaction (SIGINT, &act, NULL) < 0) {
        bail_out(EXIT_FAILURE, "set signal SIGINT");
    }
    if(sigaction (SIGTERM, &act, NULL) < 0) {
        bail_out(EXIT_FAILURE, "set signal SIGTERM");
    }

	print_commands();

	data = &sm_data;

	if((sem = sem_open(SEM_NAME, 0)) == SEM_FAILED) {
		bail_out(EXIT_FAILURE, "Semaphore %s doesn't exist", SEM_NAME);
	}

	if((sem_req = sem_open(SEM_NAME_REQ, 0)) == SEM_FAILED) {
        bail_out(EXIT_FAILURE, "Semaphore %s cannot be created", SEM_NAME_REQ);
    }

    if((sem_res = sem_open(SEM_NAME_RES, 0)) == SEM_FAILED) {
        bail_out(EXIT_FAILURE, "Semaphore %s cannot be created", SEM_NAME_RES);
    }

    allocate_resources();

	pos[0] = 0;
	pos[1] = 0;

	while(!quit && arg != 'q') {

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

			if(fgets(buffer, MAX_DATA, stdin) == NULL) {
				bail_out(EXIT_FAILURE, "read line");
			}

			for(int i = 0; buffer[i] != '\0'; i++) {
				if(buffer[i] != 'A' && buffer[i] != 'C' && buffer[i] != 'G' && buffer[i] != 'U') {
					if(memmove(&buffer[i], &buffer[i + 1], strlen(buffer) - i) == NULL) {
							bail_out(EXIT_FAILURE, "removing invalid characters");
					}
					i = -1;
				}
			}

		}

		if(arg == 'n') {

			if(sem_wait(sem) < 0) {
				if(errno == EINTR) {
					continue;
				} else {
					bail_out(EXIT_FAILURE, "waiting for semaphore");
				}
			}

			decr++;

			if(memcpy(data->payload, buffer, MAX_DATA) == NULL) {
				bail_out(EXIT_FAILURE, "copying data in shared memory");
			}

			data->pos_start = pos[0];
    		data->pos_end = pos[1];

    		if(sem_post(sem_req) < 0) {
				bail_out(EXIT_FAILURE, "increment semaphore");
			}

    		if(sem_post(sem) < 0) {
				bail_out(EXIT_FAILURE, "increment semaphore");
			}

			decr--;

			if(sem_wait(sem_res) < 0) {
				if(errno == EINTR) {
					continue;
				} else {
					bail_out(EXIT_FAILURE, "waiting for semaphore");
				}
			}

			if(sem_wait(sem) < 0) {
				if(errno == EINTR) {
					continue;
				} else {
					bail_out(EXIT_FAILURE, "waiting for semaphore");
				}
			}

			decr++;

			pos[0] = data->pos_end;
			pos[1] = data->pos_end;

			if(data->pos_start == data->pos_end) {
				fprintf(stdout, "End reached [%d/%lu], send r to reset.\n", data->pos_start, strlen(buffer));
			} else {
				fprintf(stdout, "Protein sequence found [%d/%lu] to [%d/%lu]: %s \n", data->pos_start, strlen(buffer), data->pos_end, strlen(buffer), data->payload);
			}

			if(sem_post(sem) < 0) {
				bail_out(EXIT_FAILURE, "increment semaphore");
			}

			decr--;
		
		}

		if(arg == 'r') {
			pos[0] = 0;
			pos[1] = 0;
		}

	}

	free_resources();

	exit(EXIT_SUCCESS);	
}

/*
 * @brief Signal handler function
 */
static void signal_handler(int sig) {
    (void) fprintf(stderr, "\n<interupted>\n");
    free_resources();
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
				bail_out(EXIT_FAILURE, "USAGE: %s", progname);		
		}
	}
}

/**
 *  @brief Allocates shared memory
 */
static void allocate_resources(void) {
	if ((shm_fd = shm_open(SHM_NAME, O_RDWR, PERMISSIONS)) < 0) {
        bail_out(EXIT_FAILURE, "create shared memory");
    }

    if((data = (struct sm_data *)mmap(NULL, sizeof(struct sm_data), PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0 )) == NULL) {
        bail_out(EXIT_FAILURE, "map shared memory");
    }
}

/**
 *  @brief Free allocated resources
 */
static void free_resources(void) {

	if(decr == 1) {
		if(sem_post(sem) < 0) {
			(void) fprintf(stderr, "increment semaphore\n");
		}
	}

	if(sem_close(sem) < 0) {
		(void) fprintf(stderr, "close semaphore\n");
	}

	if(sem_close(sem_req) < 0) {
		(void) fprintf(stderr, "close semaphore\n");
	}

	if(sem_close(sem_res) < 0) {
		(void) fprintf(stderr, "close semaphore\n");
	}

	if(data != NULL) {
        if(munmap(data, MAX_DATA) < 0) {
           (void) fprintf(stderr, "unmap shared memory\n");
        }
    }
    if(shm_fd != -1) {
        if(close(shm_fd) < 0) {
           (void) fprintf(stderr, "close shared memory fd\n");
        }
    }
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
