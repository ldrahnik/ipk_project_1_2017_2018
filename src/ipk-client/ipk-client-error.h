/* Name: Lukáš Drahník
 * Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 19.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#ifndef _ipk_client_error_H_
#define _ipk_client_error_H_

enum ecodes {
  EOK = 0,               // ok, even used in protocol error
  EOPT = 1,              // invalid option (option argument is missing,
                         // unknown option, unknown option character)
  EGETADDRINFO = 2,
  ESOCKET = 3,
  EBIND = 4,
  ELISTEN = 5,
  EFILE = 6,

  // protocol error codes
  EOPEN_FILE = 100,
  EHEADER = 101,
  ELOCK_FILE = 102,
  EFILE_CONTENT = 103,
  EUNKNOWN = 99
};

#endif
