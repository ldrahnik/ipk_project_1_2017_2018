/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 19.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#ifndef _ipk_client_H_
#define _ipk_client_H_

#include "ipk-client-params.h"
#include "ipk-client-error.h"
#include "../ipk-protocol/ipk-protocol.h"
#include "../ipk-file-transfer/ipk-file-transfer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>

int main(int argc, char *argv[]);
void clean(addrinfo* addrinfo, int sock, char* buffer);

#endif
