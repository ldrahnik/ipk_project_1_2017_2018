/* Name: Lukáš Drahník
 * Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 19.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#ifndef _ipk_client_params_H_
#define _ipk_client_params_H_

#include "ipk-client-error.h"

#include <string>
#include <unistd.h>

enum transfer_mode {
  READ = 0,
  WRITE = 1
};

typedef struct params {
  std::string port;                   // option p
  std::string host;                   // option h
  std::string filepath;               // option [r|w]
  int transfer_mode;                  // option [r|w] r = 0, w = 1
  int ecode;                          // error code
} TParams;

TParams getParams(int argc, char *argv[]);

#endif