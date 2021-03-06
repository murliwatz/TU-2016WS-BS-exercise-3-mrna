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
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h> 
#include <sys/mman.h>
#include "mrna-server.h"

int main(int argc, char** argv) {

	parse_args(argc, argv);

    act.sa_handler = signal_handler;
    sigemptyset (&act.sa_mask);
    act.sa_flags = 0;

    if(sigaction (SIGINT, &act, NULL) < 0) {
        bail_out(EXIT_FAILURE, "set signal SIGINT");
    }
    if(sigaction (SIGTERM, &act, NULL) < 0) {
        bail_out(EXIT_FAILURE, "set signal SIGTERM");
    }

    data = &sm_data;

    if((sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0600, 1)) == SEM_FAILED) {
        bail_out(EXIT_FAILURE, "Semaphore %s cannot be created", SEM_NAME);
    }

    if((sem_req = sem_open(SEM_NAME_REQ, O_CREAT | O_EXCL, 0600, 0)) == SEM_FAILED) {
        bail_out(EXIT_FAILURE, "Semaphore %s cannot be created", SEM_NAME_REQ);
    }

    if((sem_res = sem_open(SEM_NAME_RES, O_CREAT | O_EXCL, 0600, 0)) == SEM_FAILED) {
        bail_out(EXIT_FAILURE, "Semaphore %s cannot be created", SEM_NAME_RES);
    }

    allocate_resources();

    data->pos_start = 0;
    data->pos_end = 0;

    char *mrna = NULL;

    while(!quit) {

        if(sem_wait(sem_req) < 0) {
            if(errno == EINTR) continue;
            bail_out(EXIT_FAILURE, "waiting for semaphore");
        }

        if(sem_wait(sem) < 0) {
            if(errno == EINTR) continue;
            bail_out(EXIT_FAILURE, "waiting for semaphore");
        }

        (void) fprintf(stderr, "client has set data\n");

        int pos[2];

        (void) fprintf(stderr, "server received from client: %s\n", data->payload);

        pos[0] = data->pos_start;
        pos[1] = data->pos_end;

        if((mrna = get_mrna(data->payload, pos)) != NULL) {
            if(memcpy(data->payload, mrna, strlen(mrna) + 1) == NULL) {
                bail_out(EXIT_FAILURE, "copy to shared memory");
            }
        }

        (void) fprintf(stderr, "client sends to client %s %d %d\n", mrna, pos[0], pos[1]);


        data->pos_start = pos[0];
        data->pos_end = pos[1];

        if(sem_post(sem) < 0) {
            bail_out(EXIT_FAILURE, "increment semaphore");
        }

        if(sem_post(sem_res) < 0) {
            bail_out(EXIT_FAILURE, "increment semaphore");
        }
  
    }

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
    if ((shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, PERMISSIONS)) < 0) {
        bail_out(EXIT_FAILURE, "create shared memory");
    }

    if(ftruncate(shm_fd, sizeof(struct sm_data)) < 0) {
        bail_out(EXIT_FAILURE, "allocating memory");
    }

    if((data = (struct sm_data *)mmap(NULL, sizeof(struct sm_data), PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0 )) == NULL) {
        bail_out(EXIT_FAILURE, "map shared memory");
    }
}

/**
 *  @brief Free allocated resources
 */
static void free_resources(void) {

    if(sem_close(sem) < 0) {
       (void) fprintf(stderr, "close semaphore\n");
    }

    if(sem_close(sem_req) < 0) {
       (void) fprintf(stderr, "close semaphore\n");
    }

    if(sem_close(sem_res) < 0) {
       (void) fprintf(stderr, "close semaphore\n");
    }
    
    if(sem_unlink(SEM_NAME) < 0) {
       (void) fprintf(stderr, "unlink semaphore\n");
    }

    if(sem_unlink(SEM_NAME_REQ) < 0) {
       (void) fprintf(stderr, "unlink semaphore\n");
    }

    if(sem_unlink(SEM_NAME_RES) < 0) {
       (void) fprintf(stderr, "unlink semaphore\n");
    }

    if(data != NULL) {
        if(munmap(data, MAX_DATA) < 0) {
           (void) fprintf(stderr, "unmap shared memory\n");
        }
    }
    if(shm_unlink(SHM_NAME) < 0) {
       (void) fprintf(stderr, "unlink shared memory\n");
    }
    if(shm_fd != -1) {
        if(close(shm_fd) < 0) {
           (void) fprintf(stderr, "close shared memory fd\n");
        }
    }
}

/*
 * @brief Signal handler function
 */
static void signal_handler(int sig) {
    (void) fprintf(stderr, "\n<interupted>\n");
    quit = 1;
}

/*
 * @brief Returns a codon by bases
 */
static char get_codon(char *bases) {
    for(int i = 0; i < 3; i++) {
        if(bases[i] != 'A' && bases[i] != 'C' && bases[i] != 'G' && bases[i] != 'U') {
            bail_out(EXIT_FAILURE, "Not a correct base (%c)", bases[i]);
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
static char *get_mrna(char* dna, int* pos) {
    char* mrna_new = NULL;

    bool started = false;

    int mrna_count = 0;

    int i = pos[0];
    while(i < strlen(dna)) {
        char codon;
        if(strlen(dna + i) >= 3) {
             codon = get_codon(dna + i);
        } else {
            break;
        }
        if(codon == 'M') {
            started = true;
            pos[0] = i + 3;
        } else if(codon == '\0') {
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
                mrna_new[mrna_count++] = '\0';
                pos[1] = i;
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

    if(!started) {
        pos[0] = strlen(dna);
        pos[1] = strlen(dna);
    }

    return mrna_new;
}
