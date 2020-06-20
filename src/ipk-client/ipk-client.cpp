/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 8.3.2018
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#include "ipk-client.h"

const char *HELP_MSG = {
  "Example of usage:\n\n"
  "./ipk-client -h <host> -p <port> [-r|-w] file\n\n"
  "Options:\n"
  "-h  -- show help message\n"
  "-h <host> - hostname\n"
  "-p <port> - specification port\n"
  "[-r|-w] - file"
};

/**
 * Clean mess when is program closing successfuly or with error.
 *
 * @return void
 */
void clean() {

}

/**
 * Entry point of application.
 *
 * @param int argc
 * @param char *argv[]
 *
 * @return int
 */
int main(int argc, char *argv[]) {
  int ecode = EOK;
  int sock;
  struct addrinfo host_info;
  struct addrinfo *host_ips, *rp;
  memset(&host_info, 0, sizeof host_info);
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  ssize_t recv_len;

  // parsing parameters
  TParams params = getParams(argc, argv);
  if(params.ecode != EOK) {
    cout<<"\n"<<HELP_MSG<<endl;
    return params.ecode;
  }

  // try get file
  fstream file;
  if(params.transfer_mode == WRITE && !params.filepath.empty()) {
    file.open(params.filepath.c_str(), fstream::in | fstream::binary);
    if(!file.is_open()) {
      printError(EFILE, "Error opening file to write on server: " + params.filepath);
      return EFILE;
    }
  }
  if(params.transfer_mode == READ && !params.filepath.empty()) {
    file.open(params.filepath.c_str(), fstream::out | fstream::binary | fstream::trunc);
    if(!file.is_open()) {
      printError(EFILE, "Error opening file to write on client: " + params.filepath);
      return EFILE;
    }
  }

  // try get addrinfo
  if((getaddrinfo(params.host.c_str(), params.port.c_str(), &host_info, &host_ips)) != 0) {
    printError(EOPT, "Hostname address is not valid.");
    return EOPT;
  }

  // create socket, connect on given addres
  for (rp = host_ips; rp != NULL; rp = rp->ai_next) {
    if((sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1)
      continue;
       if(connect(sock, rp->ai_addr, rp->ai_addrlen) != -1)
         break;
       else
         close(sock);
  }

  // write
  if(params.transfer_mode == WRITE) {
    char buffer[BUFFER_SIZE];
    int len = params.filepath.length();
    long total_sent = 0;
    long file_size = 0;

    file.seekg(0, file.end);
    file_size = file.tellg();
    file.seekg(0, file.beg);

    // header
    buffer[0] = WRITE;
    memcpy(buffer+1, &len, sizeof(int));
    params.filepath.copy(buffer+1+sizeof(int), len);
    memcpy(buffer+1+sizeof(int) + len, &file_size, sizeof(long));

    // send header
    send(sock, buffer, 1 + sizeof(int)+len+sizeof(long), 0);

    // waiting on response on header
    char response = STATUS_CODE_EUNKNOWN;

    // waiting on response on header
	if((recv(sock, &response, 1, 0)) != 1)
      printError(STATUS_CODE_EHEADER, "Header was not succesfully transfered.");

    // header response
    switch(response) {
      case STATUS_CODE_EOPEN_FILE:
        printError(STATUS_CODE_EOPEN_FILE, "File can not be opened.");
        return STATUS_CODE_EOPEN_FILE;
      case STATUS_CODE_ELOCK_FILE:
        printError(STATUS_CODE_ELOCK_FILE, "File can not be locked.");
        return STATUS_CODE_ELOCK_FILE;
      case STATUS_CODE_EHEADER:
        printError(STATUS_CODE_EHEADER, "Header error.");
        return STATUS_CODE_EHEADER;
      case STATUS_CODE_OK:
        break;
      case STATUS_CODE_EUNKNOWN:
        printError(STATUS_CODE_EUNKNOWN, "Unknown response.");
        return STATUS_CODE_EUNKNOWN;
      default:
        printError(STATUS_CODE_EUNKNOWN, "Unknown response.");
        return STATUS_CODE_EUNKNOWN;
    }

    // sending file
    cout<<"Sending file: '"<<params.filepath<<"' Velikost: "<<file_size<<" B"<<endl;
    while(file.read(buffer, BUFFER_SIZE)) {
      send(sock, buffer, BUFFER_SIZE, 0);
      total_sent += file.gcount();
      cout<<file.gcount()<<" B sent. Total number of sent bytes: "<<total_sent<<" B / "<<file_size<<" B"<<endl;
    }

    send(sock, buffer, file.gcount(), 0);
    total_sent += file.gcount();
    cout<<file.gcount()<<" B sent. Total number of sent bytes: "<<total_sent<<" B / "<<file_size<<" B"<<endl;

    file.close();
  }
  // read
  else {
    char buffer[BUFFER_SIZE];
    int len = params.filepath.length();
    char response = STATUS_CODE_EUNKNOWN;

    // header
    buffer[0] = READ;
    memcpy(buffer+1, &len, sizeof(int));
    params.filepath.copy(buffer+1+sizeof(int), len);

    // send header
    send(sock, buffer, 1+sizeof(int)+len, 0);

    // waiting on response on header
    if((recv(sock, &response, 1, 0)) != 1) {
      printError(STATUS_CODE_EHEADER, "Header was not succesfully transfered.");
      return STATUS_CODE_EHEADER;
    }

    // header response
    switch(response) {
      case STATUS_CODE_EOPEN_FILE:
        printError(STATUS_CODE_EOPEN_FILE, "File can not be opened.");
        return STATUS_CODE_EOPEN_FILE;
      case STATUS_CODE_ELOCK_FILE:
        printError(STATUS_CODE_ELOCK_FILE, "File can not be locked.");
        return STATUS_CODE_ELOCK_FILE;
      case STATUS_CODE_EHEADER:
        printError(STATUS_CODE_EHEADER, "Header error.");
        return STATUS_CODE_EHEADER;
      case STATUS_CODE_OK:
        break;
      default:
        printError(STATUS_CODE_EUNKNOWN, "Unknown response.");
        return STATUS_CODE_EUNKNOWN;
    }

    // data receiving
    long total_received = 0;

    cout<<"Receiving file: '"<<params.filepath<<"'"<<endl;

    do {
      if((recv_len = recv(sock, buffer, BUFFER_SIZE, 0)) == -1) {
        printError(STATUS_CODE_EFILE_CONTENT, "Transmission content");
        return STATUS_CODE_EFILE_CONTENT;
      }

      file.write(buffer, recv_len);

      if(recv_len == 0) {
         cout<<"Transmition ended. Total number of received bytes: "<<total_received<<" B"<<endl;
         break;
      }
      total_received += recv_len;

      cout<<file.gcount()<<" B received. Total number of received bytes: "<<total_received<<" B / "<<file.gcount()<<" B"<<endl;
    } while (true);

    file.close();
  }

  close(sock);
  clean();

  return ecode;
}
