/* Name: Lukáš Drahník
 * Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 19.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#ifndef _ipk_protocol_error_H_
#define _ipk_protocol_error_H_

enum protocol_ecode {
  STATUS_CODE_OK = 0,
  STATUS_CODE_EOPEN_FILE = 100,
  STATUS_CODE_EHEADER = 101,
  STATUS_CODE_ELOCK_FILE = 102,
  STATUS_CODE_EFILE_CONTENT = 103,
  STATUS_CODE_EUNKNOWN = 99
};

#endif
