/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 26.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#include "ipk-file-transfer-error.h"

const char* getFileTransferErrorCodeMessage(int code) {
    char const *msg;
    switch(code) {
      case FILE_TRANSFER_ERECV:
        msg = "File was not succesfully received";
        break;
      case FILE_TRANSFER_ESEND:
        msg = "File was not succesfully send";
        break;
      case FILE_TRANSFER_EFILE_OPEN:
        msg = "File was not successfully opened.";
        break;
    }
    return msg;
}
