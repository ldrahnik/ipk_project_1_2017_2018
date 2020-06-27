/* Name: Lukáš Drahník
 * Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 26.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#ifndef _ipk_file_transfer_error_H_
#define _ipk_file_transfer_error_H_

enum file_transfer_ecode {
  FILE_TRANSFER_OK = 0,
  FILE_TRANSFER_ERECV = 1,
  FILE_TRANSFER_ESEND = 2,
  FILE_TRANSFER_EFILE_OPEN = 3,
};

const char* getFileTransferErrorCodeMessage(int code);

#endif
