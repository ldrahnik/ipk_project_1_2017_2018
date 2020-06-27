/* Name: Lukáš Drahník
 * Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 26.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#ifndef _ipk_file_transfer_H_
#define _ipk_file_transfer_H_

#include "ipk-file-transfer-error.h"

#include <fstream>
#include <sys/socket.h>

int sendFileFromFileStream(std::fstream& file, int sock, int chunk_size);
int receiveFileToFilePath(const char* file_path, int sock, int chunk_size);
int receiveFileToFileStream(std::fstream& file, int sock, int chunk_size);

#endif
