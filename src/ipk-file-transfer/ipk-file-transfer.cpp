/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 26.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#include "ipk-file-transfer.h"

int receiveFileToFilePath(const char* file_path, int sock, int chunk_size) {

  std::fstream file(file_path, std::fstream::out | std::fstream::binary | std::fstream::trunc);
  if(!file.is_open()) {
    return FILE_TRANSFER_EFILE_OPEN;
  }

  char *buffer = new char[chunk_size];
  ssize_t recv_len;

  while(true) {
    if((recv_len = recv(sock, buffer, chunk_size, 0)) == -1)
      return FILE_TRANSFER_ERECV;

    if(recv_len == 0)
      return FILE_TRANSFER_OK;

    file.write(buffer, recv_len);
  }

  delete[] buffer;
  file.close();

  return FILE_TRANSFER_OK;
}

int receiveFileToFileStream(std::fstream& file, int sock, int chunk_size) {

  char *buffer = new char[chunk_size];
  ssize_t recv_len;

  while(true) {
    if((recv_len = recv(sock, buffer, chunk_size, 0)) == -1)
      return FILE_TRANSFER_ERECV;

    if(recv_len == 0)
      return FILE_TRANSFER_OK;

    file.write(buffer, recv_len);
  }

  delete[] buffer;

  return FILE_TRANSFER_OK;
}

int sendFileFromFileStream(std::fstream& file, int sock, int chunk_size) {

  char *buffer = new char[chunk_size];

  while(file.read(buffer, chunk_size)) {
    if(send(sock, buffer, file.gcount(), 0) == -1) {
      return FILE_TRANSFER_ESEND;
    }
  }

  delete[] buffer;

  return FILE_TRANSFER_OK;
}
