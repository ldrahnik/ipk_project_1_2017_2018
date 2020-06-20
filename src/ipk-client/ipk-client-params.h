/* Name: Lukáš Drahník
 * Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 19.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#ifndef _ipk_client_params_H_
#define _ipk_client_params_H_

#include "ipk-client-error.h"
#include "../ipk-protocol/ipk-protocol.h"

#include <stdio.h>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct params {
  std::string port;                   // option p
  std::string host;                   // option h
  std::string filepath;               // value of option [r|w]
  int transfer_mode;                  // option [r|w] r = 0, w = 1
  int ecode;                          // error code
} TParams;

TParams getParams(int argc, char *argv[]);
int isHostValid(std::string host);

#endif
