/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 19.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#ifndef _ipk_server_H_
#define _ipk_server_H_

#include "ipk-server-params.h"
#include "ipk-server-error.h"
#include "../ipk-protocol/ipk-protocol-error.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <sys/file.h>

#include <fstream>

using namespace std;

#define MAX_CLIENTS 128
#define BUFFER_SIZE 1448

/**
 * Arguments for pthread created for each client.
 */
typedef struct pthread_args {
  struct params *params;
  int node_index;                     // start index is 0
  struct addrinfo *addrinfo;
  int sock;
} Tpthread_args;

/**
 * Transfer file modes.
 */
enum modes {
  READ = 0,
  WRITE = 1
};

void error(int code, string msg);
void catchsignal(int sig);
void clean(TParams *params, addrinfo* addrinfo, Tpthread_args* threads_args[]);
void serverError(TParams* params, int node_index, int client_sock, int code, string msg);
void* handleServer(void *threadarg);
int main(int argc, char *argv[]);

#endif
