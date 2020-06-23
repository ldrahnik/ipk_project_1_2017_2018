/* Name: Lukáš Drahník
 * Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 19.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#ifndef _ipk_server_error_H_
#define _ipk_server_error_H_

#include <string>
#include <iostream>

using namespace std;

enum server_ecode {
  EOK = 0,
  EOPT = 1,
  EGETADDRINFO = 2,
  ESOCKET = 3,
  EBIND = 4,
  ELISTEN = 5,
  EFILE = 6,
  ETHREAD = 7,
  ETHREAD_CREATE = 8,
  EALLOC = 9,
};

void printError(int code, std::string msg);

#endif
