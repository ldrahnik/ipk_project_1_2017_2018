/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 8.3.2018
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#include "ipk-server.h"

const char *HELP_MSG = {
  "Example of usage:\n\n"
  "./ipk-server [-h] [-r <number>] -p <port> \n\n"
  "Options:\n"
  "-h - show help message\n"
  "-r <number> - number of handled requests, then server ends\n"
  "-p <port> - specification port\n"
};

// when is pressed ctrl+c
static int G_break = 0;

void catchsignal(int sig) {
  if(sig == SIGINT) {
    G_break = 1;
  }
}

// free all allocated memory
void clean(TParams *params, addrinfo* addrinfo, Tpthread_args* threads_args[], int sock) {
  for(int index = 0; index < params->nodes_count; index++) {
    delete threads_args[index];
  }
  close(sock);
  freeaddrinfo(addrinfo);
}

void cleanClientThread(char* buffer, int sock) {
  if(buffer != NULL)
    free(buffer);
  close(sock);
}

void serverError(TParams* params, int node_index, int code, string msg) {
  printError(code, msg);
  cout<<"[SERVER CLIENT #"<<node_index<<"] Is leaving with error\n"<<endl;
  params->nodes_count--;
}

void serverEnd(TParams* params, int node_index) {
  cout<<"[SERVER CLIENT #"<<node_index<<"] Is leaving without error\n"<<endl;
  params->nodes_count--;
}

void* handleClientThread(void *threadarg) {
  uint8_t ecode = EOK;

  // thread args
  struct pthread_args *pthread_args = (struct pthread_args *) threadarg;
  TParams* params = (TParams*) pthread_args->params;
  int sock = pthread_args->sock;
  int node_index = pthread_args->node_index;

  // buffer's
  char* buffer = NULL;
  char* file_path = NULL;

  // alloc buffer's
  buffer = (char*)malloc(sizeof(char) * IP_MAXPACKET);
  if(buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    serverError(params, node_index, EALLOC, "Allocation fails.");
    pthread_exit(NULL);
  }

  cout<<"[SERVER CLIENT #"<<node_index<<"] Is starting\n"<<endl;

  // incoming header
  if(recv(sock, buffer, IP_MAXPACKET, 0) < 0) {
    ecode = STATUS_CODE_ERECV_HEADER;
    send(sock, &ecode, sizeof(ecode), 0);
    cleanClientThread(buffer, sock);
    serverError(params, node_index, ecode, getStatusCodeMessage(ecode));
    pthread_exit(NULL);
  }
  Protocol_header* header = (Protocol_header*) buffer;

  // file_path follows-up header
  file_path = (char*) (buffer + sizeof(Protocol_header));

  // root of server (current working directory)
  char cwd[PATH_MAX];
  getcwd(cwd, PATH_MAX);

  // write
  if(header->transfer_mode == WRITE) {

    long file_size;
    file_size = ntohs(header->file_size);

    cout<<"[SERVER CLIENT #"<<node_index<<"] Wants write a file. Sent filename: '"<<basename(file_path)<<"', Size: "<<file_size<<" B"<<endl;

    // open file
    fstream file (std::string(cwd) + std::string("/") + basename(file_path), fstream::out | fstream::binary | fstream::trunc);
    if(!file.is_open()) {
      ecode = STATUS_CODE_EOPEN_FILE;
      send(sock, &ecode, sizeof(ecode), 0);
      cleanClientThread(buffer, sock);
      serverError(params, node_index, ecode, getStatusCodeMessage(ecode));
      file.close();
      pthread_exit(NULL);
    }

    // opened => OK
    send(sock, &ecode, sizeof(ecode), 0);

    // receive file
    if((ecode = receiveFileToFileStream(file, sock, BUFFER_SIZE))) {
      cleanClientThread(buffer, sock);
      serverError(params, node_index, ecode, getFileTransferErrorCodeMessage(ecode));
      file.close();
      pthread_exit(NULL);
    } else {
      cout<<"[SERVER CLIENT #"<<node_index<<"] Successfully received file."<<endl;
    }

    file.close();
  }
  // read
  else if (header->transfer_mode == READ) {
    cout<<"[SERVER CLIENT #"<<node_index<<"] wants read a file. Received filename: '"<<basename(file_path)<<"'"<<endl;

    // open file
    fstream file (std::string(cwd) + std::string("/") + basename(file_path), fstream::in | fstream::binary);
    if(!file.is_open()) {
      ecode = STATUS_CODE_EOPEN_FILE;
      send(sock, &ecode, sizeof(ecode), 0);
      cleanClientThread(buffer, sock);
      serverError(params, node_index, ecode, getStatusCodeMessage(ecode));
      pthread_exit(NULL);
    }

    // opened => OK
    send(sock, &ecode, sizeof(ecode), 0);

    // send file
    if((ecode = sendFileFromFileStream(file, sock, BUFFER_SIZE))) {
      cleanClientThread(buffer, sock);
      serverError(params, node_index, ecode, getFileTransferErrorCodeMessage(ecode));
      file.close();
      pthread_exit(NULL);
    } else {
      cout<<"[SERVER CLIENT #"<<node_index<<"] Successfully sent file."<<endl;
    }

    file.close();

  // else
  } else {
    ecode = STATUS_CODE_ERECV_HEADER_TRANSFER_MODE;
    send(sock, &ecode, sizeof(ecode), 0);
    cleanClientThread(buffer, sock);
    serverError(params, node_index, ecode, getStatusCodeMessage(ecode));
    pthread_exit(NULL);
  }

  // clean
  cleanClientThread(buffer, sock);
  serverEnd(params, node_index);

  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int ecode = EOK;
  int sock = -1;
  int client_sock = -1;
  struct addrinfo* results;
  struct addrinfo hints;
  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  pthread_t threads[MAX_CLIENTS];
  Tpthread_args* threads_args[MAX_CLIENTS];
  int requests_count = 0;

  // get args
  TParams params = getParams(argc, argv);
  if(params.show_help_message) {
    cout<<HELP_MSG<<endl;
    return params.ecode;
  }
  if(params.ecode != EOK) {
    cout<<"\n"<<HELP_MSG<<endl;
    clean(&params, results, threads_args, sock);
    return params.ecode;
  }

  // addrinfo from given port
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  // get addrinfo
  if(getaddrinfo(NULL, params.port.c_str(), &hints, &results) != 0) {
    printError(EGETADDRINFO, "Host is not valid.\n");
    clean(&params, results, threads_args, sock);
    return EGETADDRINFO;
  }

  // create socket
  if((sock = socket(results->ai_family, results->ai_socktype, results->ai_protocol)) == -1) {
    printError(ESOCKET, "Socket can not be created.\n");
    clean(&params, results, threads_args, sock);
    return ESOCKET;
  }

  // bind socket
  if(bind(sock, results->ai_addr, results->ai_addrlen) != 0) {
    printError(EBIND, "Bind failed.");
    clean(&params, results, threads_args, sock);
    return EBIND;
  }

  // listen
  if(listen(sock, MAX_CLIENTS) < 0) {
    printError(ELISTEN, "Listen failed.");
    clean(&params, results, threads_args, sock);
    return ELISTEN;
  }

  cout<<"[SERVER] Server is running on port: "<<params.port<<"\n"<<endl;

  while(true) {


    // server already handled number of requested requests
    if(params.requests_count != -1 && requests_count == params.requests_count)
      break;

    // create client socket
    if((client_sock = accept(sock, (struct sockaddr *) &client_addr, &addr_size)) <= 0)
      continue;

    cout<<"[SERVER] Joined client. Started thread with number: "<<params.nodes_count<<"\n"<<endl;

    requests_count++;

    // create Tpthread_args
    Tpthread_args* threadarg = new Tpthread_args();
    threadarg->params = &params;
    threadarg->node_index = params.nodes_count;
    threads_args[params.nodes_count] = threadarg;
    threadarg->sock = client_sock;

    // pthread per client
    if(pthread_create(&threads[params.nodes_count], NULL, handleClientThread, (void *) threadarg) != 0) {
      printError(ETHREAD, "Unable to create thread.\n");
      return ETHREAD;
    }

    // increment client pthreads
    params.nodes_count++;
  }

  // wait for all child node's
  for(int index = 0; index < params.nodes_count; index++)
    pthread_join(threads[index], NULL);

  // clean
  clean(&params, results, threads_args, sock);

  return ecode;
}
