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
void clean(TParams *params, addrinfo* addrinfo, Tpthread_args* threads_args[]) {
  for(int index = 0; index < params->nodes_count; index++) {
    close(threads_args[index]->sock);
    free(threads_args[index]->addrinfo);
    delete threads_args[index];
  }
  freeaddrinfo(addrinfo);
}

void serverError(TParams* params, int node_index, int client_sock, int code, string msg) {
  printError(code, msg);
  cout<<"[CLIENT #"<<node_index<<"] Is leaving with error\n"<<endl;
  close(client_sock);
  params->nodes_count--;
}

void serverEnd(TParams* params, int node_index, int client_sock) {
  cout<<"[CLIENT #"<<node_index<<"] Is leaving without error\n"<<endl;
  close(client_sock);
  params->nodes_count--;
}

void* handleServer(void *threadarg) {
  int ecode = EOK;

  // thread args
  struct pthread_args *pthread_args = (struct pthread_args *) threadarg;
  TParams* params = (TParams*) pthread_args->params;
  int sock = pthread_args->sock;
  int node_index = pthread_args->node_index;

  // buffer's
  char* buffer;
  char* file_path;

  buffer = (char*)malloc(sizeof(char) * IP_MAXPACKET);
  if(buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    serverError(params, node_index, sock, EALLOC, "Alloc error.");
    pthread_exit(NULL);
  }

  cout<<"[CLIENT #"<<node_index<<"] Is starting\n"<<endl;

  // incoming header
  if(recv(sock, buffer, IP_MAXPACKET, 0) < 0) {
    ecode = STATUS_CODE_EHEADER;
    send(sock, &ecode, 1, 0);
    free(buffer);
    serverError(params, node_index, sock, STATUS_CODE_EHEADER, "Header error.");
    pthread_exit(NULL);
  }
  Protocol_header* header = (Protocol_header*) buffer;

  // file_path follows-up header
  file_path = (char*)malloc(sizeof(char) * (header->file_path_length));
  if(file_path == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    serverError(params, node_index, sock, EALLOC, "Alloc error.");
    pthread_exit(NULL);
  }
  file_path = (char*) (buffer + sizeof(Protocol_header));

  // write
  if(header->transfer_mode == WRITE) {

    cout<<"[CLIENT #"<<node_index<<"] Wants write a file. Sent filename: '"<<basename(file_path)<<"', Size: "<<header->file_size<<" B"<<endl;

    // open file
    ofstream output_file;
    output_file.open(basename(file_path), fstream::out | fstream::binary | fstream::trunc);
    if(!output_file.is_open()) {
      ecode = STATUS_CODE_EOPEN_FILE;
      send(sock, &ecode, 1, 0);
      free(buffer);
      serverError(params, node_index, sock, STATUS_CODE_EOPEN_FILE, "Server can not open: " + std::string(file_path));
      pthread_exit(NULL);
    }

    // lock file
    /*if(flock(output_file.filedesc(), LOCK_EX | LOCK_NB) != 0) {
      ecode = ELOCK_FILE;
      send(sock, &ecode, 1, 0);
      serverError(params, node_index, sock, ELOCK_FILE, "Server can not lock: " + basename(filepath));
    }*/

    // opened, locked => OK
    send(sock, &ecode, 1, 0);

    long total_received = 0;
    ssize_t recv_len;
    do {
      if((recv_len = recv(sock, buffer, BUFFER_SIZE, 0)) == -1) {
        ecode = STATUS_CODE_EFILE_CONTENT;
        send(sock, &ecode, 1, 0);
        serverError(params, node_index, sock, STATUS_CODE_EFILE_CONTENT, "Error during data of file transmission");
        pthread_exit(NULL);
      }
      //cout<<"[CLIENT #"<<node_index<<"]"<<output_file.gcount()<<" B received. Total number of received bytes: "<<total_received<<" B / "<<filepath_len<<" B"<<endl;

      total_received += recv_len;

      output_file.write(buffer, recv_len);

      if(recv_len == 0) {
        if(header->file_size != total_received) {
          cout<<"[CLIENT #"<<node_index<<"] Transmition ended with error about file content size. Total number of received bytes: "<<total_received<<" B"<<endl;
          ecode = STATUS_CODE_EFILE_CONTENT;
          send(sock, &ecode, 1, 0);
          serverError(params, node_index, sock, STATUS_CODE_EFILE_CONTENT, "Server can not open: " + std::string(file_path));
          pthread_exit(NULL);
        } else {
          cout<<"[CLIENT #"<<node_index<<"] Transmition ended successfully. Total number of received bytes: "<<total_received<<" B"<<endl;
          send(sock, &ecode, 1, 0);
        }
      }
    } while (recv_len > 0);

    // close file
    output_file.close();

    // unlock file
    //flock(output_file.filedesc(), LOCK_UN);
  }
  // read
  else if (header->transfer_mode == READ) {
    cout<<"[CLIENT #"<<node_index<<"] wants read a file. Received filename: '"<<basename(file_path)<<"'"<<endl;

    // open file
    ifstream input_file;
    input_file.open(file_path, fstream::in | fstream::binary);
    if(!input_file.is_open()) {
      ecode = STATUS_CODE_EOPEN_FILE;
      send(sock, &ecode, 1, 0);
      serverError(params, node_index, sock, STATUS_CODE_EOPEN_FILE, "Server can not open: " + std::string(file_path));
      pthread_exit(NULL);
    }

    // lock file
    /*if(flock(input_file.filedesc(), LOCK_SH | LOCK_NB) != 0) {
      ecode = ELOCK_FILE;
      send(sock, &ecode, 1, 0);
      serverError(params, node_index, sock, ELOCK_FILE, "Server can not lock: " + std::string(filepath));
      pthread_exit(NULL);
    }*/

    // opened, locked => OK
    send(sock, &ecode, 1, 0);

    long total_sent = 0;
    long file_size = 0;

    input_file.seekg(0, input_file.end);
    file_size = input_file.tellg();
    input_file.seekg(0, input_file.beg);

    while(input_file.read(buffer, BUFFER_SIZE)) {
      send(sock, buffer, BUFFER_SIZE, 0);
      total_sent += input_file.gcount();
      cout<<"[CLIENT #"<<node_index<<"] "<<input_file.gcount()<<" B sent. Total number of sent bytes: "<<total_sent<<" B / "<<file_size<<" B"<<endl;
    }

    send(sock, buffer, input_file.gcount(), 0);
    total_sent += input_file.gcount();
    cout<<"[CLIENT #"<<node_index<<"] Transmition ended. Total number of sent bytes: "<<total_sent<< " B / "<<file_size<<" B"<<endl;

    // close file
    input_file.close();

    // unlock file
    //flock(input_file.filedesc(), LOCK_UN);

  // else
  } else {
    ecode = STATUS_CODE_EHEADER;
    send(sock, &ecode, 1, 0);
    serverError(params, node_index, sock, ecode, "Header error. Transfer mode could not be recognized.");
    free(buffer);
    pthread_exit(NULL);
  }

  // clean
  free(buffer);
  serverEnd(params, node_index, sock);

  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int ecode = EOK;
  int sock, client_sock;
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
    clean(&params, results, threads_args);
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
    clean(&params, results, threads_args);
    return EGETADDRINFO;
  }

  // create socket
  if((sock = socket(results->ai_family, results->ai_socktype, results->ai_protocol)) == -1) {
    printError(ESOCKET, "Socket can not be created.\n");
    clean(&params, results, threads_args);
    return ESOCKET;
  }

  // bind socket
  if(bind(sock, results->ai_addr, results->ai_addrlen) != 0) {
    printError(EBIND, "Bind failed.");
    clean(&params, results, threads_args);
    return EBIND;
  }

  // listen
  if(listen(sock, MAX_CLIENTS) < 0) {
    printError(ELISTEN, "Listen failed.");
    clean(&params, results, threads_args);
    return ELISTEN;
  }

  cout<<"[SERVER] Server is running on port: "<<params.port<<"\n"<<endl;

  while(true) {

    // CTRL+C handler
    if(G_break == 1)
      break;

    // server already handled number of requested requests
    if(params.requests_count != -1 && requests_count > params.requests_count)
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
    threadarg->addrinfo = results;
    threads_args[params.nodes_count] = threadarg;
    threadarg->sock = client_sock;

    // pthread per client
    if(pthread_create(&threads[params.nodes_count], NULL, handleServer, (void *) threadarg) != 0) {
      printError(ETHREAD, "Unable to create thread.\n");
      return ETHREAD;
    }

    // increment client pthreads
    params.nodes_count++;

    break;
  }

  // wait for all child node's
  for(int index = 0; index < params.nodes_count; index++)
    pthread_join(threads[index], NULL);

  // clean
  clean(&params, results, threads_args);

  return ecode;
}
