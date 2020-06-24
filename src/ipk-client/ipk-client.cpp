/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 8.3.2018
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#include "ipk-client.h"

const char *HELP_MSG = {
  "Example of usage:\n\n"
  "./ipk-client -h <host> -p <number> -r|-w <file>\n\n"
  "Options:\n"
  "-h <host> - hostname\n"
  "-p <number> - port\n"
  "-r|-w <file> - read/write file"
};

// free all allocated memory
void clean(addrinfo* addrinfo, int sock, char* buffer, fstream& file) {
  if(buffer != NULL)
    free(buffer);
  close(sock);
  freeaddrinfo(addrinfo);
  file.close();
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
  int sock = -1;
  char* buffer = NULL;
  struct addrinfo host_info;
  struct addrinfo *host_ips, *rp;
  memset(&host_info, 0, sizeof host_info);
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  ssize_t recv_len;
  fstream file;

  // parsing parameters
  TParams params = getParams(argc, argv);
  if(params.ecode != EOK) {
    cout<<"\n"<<HELP_MSG<<endl;
    return params.ecode;
  }

  // try get addrinfo
  if((getaddrinfo(params.host.c_str(), params.port.c_str(), &host_info, &host_ips)) != 0) {
    printError(EOPT, "Hostname address is not valid.");
    clean(host_ips, sock, buffer, file);
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

  // filepath length
  int file_path_length = 0;
  file_path_length = params.filepath.length();

  buffer = (char*)malloc(sizeof(Protocol_header) + file_path_length + 1);
  if(buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    clean(host_ips, sock, buffer, file);
    return EALLOC;
  }

  // header
  Protocol_header* header = (Protocol_header*)buffer;
  header->file_path_length = file_path_length;

  // file_path follows-up header
  params.filepath.copy(buffer + sizeof(Protocol_header), file_path_length);
  buffer[sizeof(Protocol_header) + file_path_length] = '\0';

  // header response
  char response = STATUS_CODE_EUNKNOWN;

  // write
  if(params.transfer_mode == WRITE) {

    long file_size = 0;

    file.seekg(0, file.end);
    file_size = file.tellg();
    file.seekg(0, file.beg);

    // header
    header->transfer_mode = WRITE;
    header->file_size = file_size;

    // send header
    if((send(sock, buffer, sizeof(Protocol_header) + file_path_length + 1, 0)) == -1) {
      printError(ESEND, "Header was not succesfully sent.");
      clean(host_ips, sock, buffer, file);
      return ESEND;
    }

    // waiting on header response
	if((recv(sock, &response, 1, 0)) == -1) {
      printError(STATUS_CODE_EHEADER, "Header was not succesfully transfered.");
      clean(host_ips, sock, buffer, file);
      return STATUS_CODE_EHEADER;
    }
    switch(response) {
      case STATUS_CODE_EOPEN_FILE:
        printError(STATUS_CODE_EOPEN_FILE, "File can not be opened.");
        clean(host_ips, sock, buffer, file);
        return STATUS_CODE_EOPEN_FILE;
      case STATUS_CODE_ELOCK_FILE:
        printError(STATUS_CODE_ELOCK_FILE, "File can not be locked.");
        clean(host_ips, sock, buffer, file);
        return STATUS_CODE_ELOCK_FILE;
      case STATUS_CODE_EHEADER:
        printError(STATUS_CODE_EHEADER, "Header error.");
        clean(host_ips, sock, buffer, file);
        return STATUS_CODE_EHEADER;
      case STATUS_CODE_OK:
        break;
      case STATUS_CODE_EUNKNOWN:
        printError(STATUS_CODE_EUNKNOWN, "Unknown response.");
        clean(host_ips, sock, buffer, file);
        return STATUS_CODE_EUNKNOWN;
      default:
        printError(STATUS_CODE_EUNKNOWN, "Unknown response.");
        clean(host_ips, sock, buffer, file);
        return STATUS_CODE_EUNKNOWN;
    }

    cout<<"Sending file: '"<<params.filepath<<"' Velikost: "<<file_size<<" B"<<endl;

    file.open(params.filepath.c_str(), fstream::in | fstream::binary);
    if(!file.is_open()) {
      printError(EFILE, "Error opening file to write on server: " + params.filepath);
      clean(host_ips, sock, buffer, file);
      return EFILE;
    }

    // sending file
    long total_sent = 0;
    while(file.read(buffer, BUFFER_SIZE)) {
      send(sock, buffer, BUFFER_SIZE, 0);
      total_sent += file.gcount();
      cout<<file.gcount()<<" B sent. Total number of sent bytes: "<<total_sent<<" B / "<<file_size<<" B"<<endl;
    }

    send(sock, buffer, file.gcount(), 0);
    total_sent += file.gcount();
    cout<<file.gcount()<<" B sent. Total number of sent bytes: "<<total_sent<<" B / "<<file_size<<" B"<<endl;
  }
  // read
  else {

    // header
    header->transfer_mode = READ;
    header->file_size = 0;

    // send header
    if((send(sock, buffer, sizeof(Protocol_header) + file_path_length, 0)) == -1) {
      printError(ESEND, "Header was not succesfully sent.");
      clean(host_ips, sock, buffer, file);
      return ESEND;
    }

    // waiting on header response
    if((recv(sock, &response, 1, 0)) != 1) {
      printError(STATUS_CODE_EHEADER, "Header was not succesfully transfered.");
      clean(host_ips, sock, buffer, file);
      return STATUS_CODE_EHEADER;
    }
    switch(response) {
      case STATUS_CODE_EOPEN_FILE:
        printError(STATUS_CODE_EOPEN_FILE, "File can not be opened.");
        clean(host_ips, sock, buffer, file);
        return STATUS_CODE_EOPEN_FILE;
      case STATUS_CODE_ELOCK_FILE:
        printError(STATUS_CODE_ELOCK_FILE, "File can not be locked.");
        clean(host_ips, sock, buffer, file);
        return STATUS_CODE_ELOCK_FILE;
      case STATUS_CODE_EHEADER:
        printError(STATUS_CODE_EHEADER, "Header error.");
        clean(host_ips, sock, buffer, file);
        return STATUS_CODE_EHEADER;
      case STATUS_CODE_OK:
        break;
      case STATUS_CODE_EUNKNOWN:
        printError(STATUS_CODE_EUNKNOWN, "Unknown response.");
        clean(host_ips, sock, buffer, file);
        return STATUS_CODE_EUNKNOWN;
      default:
        printError(STATUS_CODE_EUNKNOWN, "Unknown response.");
        clean(host_ips, sock, buffer, file);
        return STATUS_CODE_EUNKNOWN;
    }

    file.open(basename(params.filepath.c_str()), fstream::out | fstream::binary | fstream::trunc);
    if(!file.is_open()) {
      printError(EFILE, "Error opening file to write on client: " + params.filepath);
      clean(host_ips, sock, buffer, file);
      return EFILE;
    }

    cout<<"Receiving file: '"<<params.filepath<<"'"<<endl;

    // try get file
    file.open(basename(params.filepath.c_str()), fstream::out | fstream::binary | fstream::trunc);
    if(!file.is_open()) {
      printError(EFILE, "Error opening file to write on client: " + params.filepath);
      clean(host_ips, sock, buffer, file);
      return EFILE;
    }

    // receiving file
    long total_received = 0;
    do {
      if((recv_len = recv(sock, buffer, BUFFER_SIZE, 0)) == -1) {
        printError(STATUS_CODE_EFILE_CONTENT, "Transmission content");
        clean(host_ips, sock, buffer, file);
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
  }

  // clean
  clean(host_ips, sock, buffer, file);

  return ecode;
}
