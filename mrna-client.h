/**
 * @file mrna-client.h
 * @brief header file for the implementation of mrna-client
 * @author Paul Pröll, 1525669
 * @date 2016-11-15
 */

#ifndef MRNA_CLIENT_H_
#define MRNA_CLIENT_H_

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
static int decr = 0;

/** struct for shared memory */
static struct sm_data {
    int pos_start;
    int pos_end;
    char payload[MAX_DATA];
} sm_data;

static struct sm_data *data;

static char buffer[MAX_DATA];
static int pos[2];

/** name of this program */
static char* progname;

static void bail_out(int exitcode, const char *fmt, ...);

static void parse_args(int argc, char** argv);

static void print_commands(void);

static void signal_handler(int sig);

static void allocate_resources(void);

static void free_resources(void);

#endif
