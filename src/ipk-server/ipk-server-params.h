/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 19.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#ifndef _ipk_server_params_H_
#define _ipk_server_params_H_

#include "ipk-server-error.h"

#include <string>
#include <unistd.h>

typedef struct params {
  std::string port;
  int show_help_message;
  int ecode;
  int nodes_count;
  int requests_count;
} TParams;

TParams getParams(int argc, char *argv[]);

#endif
