/**
 * @file mrna-server.h
 * @brief header file for the implementation of mrna-server
 * @author Paul Pröll, 1525669
 * @date 2016-11-15
 */

#ifndef MRNA_SERVER_H_
#define MRNA_SERVER_H_

static void bail_out(int exitcode, const char *fmt, ...);

static void parse_args(int argc, char** argv);

static void free_resources(void);

static char get_codon(char *bases);

static char *get_mrna(char* dna, int* pos);

#endif