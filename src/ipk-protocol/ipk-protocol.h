/* Name: Lukáš Drahník
 * Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 19.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#ifndef _ipk_protocol_H_
#define _ipk_protocol_H_

#include <stdint.h>
#include <cstdint>
 
#define BUFFER_SIZE 1448

#pragma pack(push, 1)
typedef struct protocol_header {
  uint8_t transfer_mode :1;
  int file_path_length;
  long file_size;
} Protocol_header;
// char *file_path; (variable length)
#pragma pack(pop)

enum protocol_transfer_mode {
  READ = 0,
  WRITE = 1
};

enum protocol_status_code: uint8_t {
  STATUS_CODE_OK = 0,
  STATUS_CODE_EOPEN_FILE = 100,
  STATUS_CODE_ERECV_HEADER = 101,
  STATUS_CODE_ERECV_HEADER_TRANSFER_MODE = 102,
  STATUS_CODE_ERECV_FILE = 103,
  STATUS_CODE_EFILE_CONTENT = 104,
  STATUS_CODE_EUNKNOWN = 99
};

const char* getStatusCodeMessage(int code);

#endif
