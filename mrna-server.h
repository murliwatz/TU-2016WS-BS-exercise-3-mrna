/**
 * @file mrna-server.h
 * @brief header file for the implementation of mrna-server
 * @author Paul Pröll, 1525669
 * @date 2016-01-05
 */

#ifndef MRNA_SERVER_H_
#define MRNA_SERVER_H_

#define SEM_NAME "/1525669"
#define SEM_NAME_REQ "/1525669_req"
#define SEM_NAME_RES "/1525669_res"
#define SHM_NAME "/1525669"
#define MAX_DATA (4096)
#define PERMISSIONS (0600)

static int shm_fd = -1;

static struct sigaction act;

static sem_t *sem;
static sem_t *sem_req;
static sem_t *sem_res;

volatile sig_atomic_t quit = 0;

static struct sm_data *data;

/** struct for shared memory */
static struct sm_data {
    unsigned int pos_start;
    unsigned int pos_end;
    char payload[MAX_DATA];
} sm_data;

/** name of this program */
static char* progname; 

static void bail_out(int exitcode, const char *fmt, ...);

static void parse_args(int argc, char** argv);

static void allocate_resources(void);

static void free_resources(void);

static void signal_handler(int sig);

static char get_codon(char *bases);

static char *get_mrna(char* dna, int* pos);

#endif
