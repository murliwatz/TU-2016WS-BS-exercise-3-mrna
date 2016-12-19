/**
 * @file websh.h
 * @brief header file for the implementation of mrna-client
 * @author Paul Pröll, 1525669
 * @date 2016-11-15
 */

#ifndef MRNA_CLIENT_H_
#define MRNA_CLIENT_H_

static void bail_out(int exitcode, const char *fmt, ...);

static void parse_args(int argc, char** argv);

static void print_commands(void);

static void free_resources(void);

#endif