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
void clean(addrinfo* addrinfo, int sock, char* buffer) {
  if(buffer != NULL)
    free(buffer);
  close(sock);
  freeaddrinfo(addrinfo);
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

  // parsing parameters
  TParams params = getParams(argc, argv);
  if(params.ecode != EOK) {
    cout<<"\n"<<HELP_MSG<<endl;
    return params.ecode;
  }

  // try get addrinfo
  if((getaddrinfo(params.host.c_str(), params.port.c_str(), &host_info, &host_ips)) != 0) {
    printError(EOPT, "Hostname address is not valid.");
    clean(host_ips, sock, buffer);
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
    clean(host_ips, sock, buffer);
    return EALLOC;
  }

  // header
  Protocol_header* header = (Protocol_header*)buffer;
  header->file_path_length = htons(file_path_length);

  // file_path follows-up header
  params.filepath.copy(buffer + sizeof(Protocol_header), file_path_length);
  buffer[sizeof(Protocol_header) + file_path_length] = '\0';

  // header response
  char response;

  // root of client (current working directory)
  char cwd[PATH_MAX];
  getcwd(cwd, PATH_MAX);

  // write
  if(params.transfer_mode == WRITE) {

    // open file
    fstream file (params.filepath.c_str(), fstream::in | fstream::binary);
    if(!file.is_open()) {
      printError(EFILE, "Error opening file to write on server: " + params.filepath);
      clean(host_ips, sock, buffer);
      return EFILE;
    }

    // file size (is required have opened file)
    long file_size = 0;
    file.seekg(0, file.end);
    file_size = file.tellg();
    file.seekg(0, file.beg);

    // header
    header->transfer_mode = WRITE;
    header->file_size = htons(file_size);

    // send header
    if((send(sock, buffer, sizeof(Protocol_header) + file_path_length + 1, 0)) == -1) {
      printError(ESEND, "Header was not succesfully sent.");
      clean(host_ips, sock, buffer);
      file.close();
      return ESEND;
    }

    // receive header response
	if((recv(sock, &response, 1, 0)) == -1) {
      printError(ERECV, "Header response was not succesfully received.");
      clean(host_ips, sock, buffer);
      file.close();
      return ERECV;
    }
    if(response) {
      printError(response, getStatusCodeMessage(response));
      clean(host_ips, sock, buffer);
      file.close();
      return response;
    }

    // send file
    if((ecode = sendFileFromFileStream(file, sock, BUFFER_SIZE))) {
      printError(ecode, getFileTransferErrorCodeMessage(ecode));
      clean(host_ips, sock, buffer);
      file.close();
      return ecode;
    } else {
      cout<<"[CLIENT] Successfully sent file: '"<<params.filepath<<"' with size: "<<file_size<<" B"<<endl;
    }

    file.close();
  }
  // read
  else {

    // header
    header->transfer_mode = READ;
    header->file_size = htons(0);

    // send header
    if((send(sock, buffer, sizeof(Protocol_header) + file_path_length + 1, 0)) == -1) {
      printError(ESEND, "Header was not succesfully sent.");
      clean(host_ips, sock, buffer);
      return ESEND;
    }

    // receive header response
    if((recv(sock, &response, 1, 0)) != 1) {
      printError(ERECV, "Header response was not succesfully received.");
      clean(host_ips, sock, buffer);
      return ERECV;
    }
    if(response) {
      printError(response, getStatusCodeMessage(response));
      clean(host_ips, sock, buffer);
      return response;
    }

    // receive file
    if((ecode = receiveFileToFilePath(params.filepath.c_str(), sock, BUFFER_SIZE))) {
      printError(ecode, getFileTransferErrorCodeMessage(ecode));
      clean(host_ips, sock, buffer);
      return ecode;
    } else {
      cout<<"[CLIENT] Successfully received file: '"<<params.filepath.c_str()<<"'"<<endl;
    }
  }

  // clean
  clean(host_ips, sock, buffer);

  return ecode;
}
