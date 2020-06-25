/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 8.3.2018
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#include "ipk-protocol.h"

const char* getStatusCodeMessage(int code) {
    char const *msg;
    switch(code) {
      case STATUS_CODE_EOPEN_FILE:
        msg = "File can not be opened.";
        break;
      case STATUS_CODE_ERECV_HEADER:
        msg = "Header was not succesfully received.";
        break;
      case STATUS_CODE_ERECV_HEADER_TRANSFER_MODE:
        msg = "Header transfer mode has not been recognized.";
        break;
      case STATUS_CODE_ERECV_FILE:
        msg = "File was not succesfully received.";
        break;
      case STATUS_CODE_EFILE_CONTENT:
        msg = "File content was not successfully transfered";
        break;
      case STATUS_CODE_EUNKNOWN:
        msg = "Unknown error.";
        break;
      default:
        msg = getStatusCodeMessage(STATUS_CODE_EUNKNOWN);
        break;
    }
    return msg;
}
