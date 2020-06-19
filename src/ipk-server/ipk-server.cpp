/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 8.3.2018
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#include "ipk-server.h"

/**
 * When is pressed ctrl+c.
 */
static int G_break = 0;

/**
 * Help message.
 */
const char *HELP_MSG = {
  "Example of usage:\n\n"
  "./ipk-server [-h] [-r <number>] -p <port> \n\n"
  "Options:\n"
  "-h - show help message\n"
  "-r <number> - number of handled requests, then server ends\n"
  "-p <port> - specification port\n"
};

/**
 * Print error message.
 */
void error(int code, string msg) {
	cerr<<msg<< endl;
  exit(code);
}

/**
 * Signal handler.
 */
void catchsignal(int sig) {
  if(sig == SIGINT) {
    G_break = 1;
  }
}

/**
 * Clean mess when is program closing successfuly or with error.
 *
 * @return void
 */
void clean(TParams *params, addrinfo* addrinfo, Tpthread_args* threads_args[]) {
  for(int index = 0; index < params->nodes_count; index++) {
    close(threads_args[index]->sock);
    free(threads_args[index]->addrinfo);
    delete threads_args[index];
  }
  freeaddrinfo(addrinfo);
}

void serverError(TParams* params, int node_index, int client_sock, int code, string msg) {
  cerr<<code<<msg<<endl;
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

  struct pthread_args *pthread_args = (struct pthread_args *) threadarg;
  //struct node node = (struct node) pthread_args->params->nodes[pthread_args->node_index];
  //struct addrinfo *addrinfo = pthread_args->addrinfo;
  TParams* params = (TParams*) pthread_args->params;
  int client_sock = pthread_args->sock;
  int node_index = pthread_args->node_index;
  ssize_t recv_len;
  char buffer[BUFFER_SIZE];

  cout<<"[CLIENT #"<<node_index<<"] Is starting\n"<<endl;

  recv_len = recv(client_sock, buffer, 1+sizeof(int), 0);

  if(recv_len != 1 + sizeof(int)) {
    ecode = STATUS_CODE_EHEADER;
    send(client_sock, &ecode, 1, 0);
    serverError(params, node_index, client_sock, STATUS_CODE_EHEADER, "Header error.");
    pthread_exit(NULL);
  }

  char mode = buffer[0];
  int filepath_len;
  memcpy(&filepath_len, buffer+1, sizeof(int));

  // filename
  recv_len = recv(client_sock, buffer, filepath_len, 0);

  if(recv_len != filepath_len) {
    ecode = STATUS_CODE_EHEADER;
    send(client_sock, &ecode, 1, 0);
    serverError(params, node_index, client_sock, STATUS_CODE_EHEADER, "Header error.");
    pthread_exit(NULL);
  }

  // filename
  buffer[filepath_len] = '\0';
  string filepath = buffer;

  // write
  if(mode == WRITE) {
    // filesize
    recv_len = recv(client_sock, buffer, sizeof(long), 0);

    if(recv_len != sizeof(long)) {
      ecode = STATUS_CODE_EHEADER;
      send(client_sock, &ecode, 1, 0);
      serverError(params, node_index, client_sock, STATUS_CODE_EHEADER, "Header error.");
      pthread_exit(NULL);
    }

    long file_size;
    memcpy(&file_size, buffer, sizeof(long));

    //cout<<"[CLIENT #"<<node_index<<"] wants write a file. Sent filename: '"<<basename(filepath.c_str())<<"', Size: "<<file_size<<" B"<<endl;
    cout<<"[CLIENT #"<<node_index<<"] Wants write a file. Sent filename: '"<<basename(filepath.c_str())<<"'"<<endl;

    // open file
    ofstream output_file;
    output_file.open(basename(filepath.c_str()), fstream::out | fstream::binary | fstream::trunc);
    if(!output_file.is_open()) {
      ecode = STATUS_CODE_EOPEN_FILE;
      send(client_sock, &ecode, 1, 0);
      serverError(params, node_index, client_sock, STATUS_CODE_EOPEN_FILE, "Server can not open: " + filepath);
      pthread_exit(NULL);
    }

    // lock file
    /*if(flock(output_file.filedesc(), LOCK_EX | LOCK_NB) != 0) {
      ecode = ELOCK_FILE;
      send(client_sock, &ecode, 1, 0);
      serverError(params, node_index, client_sock, ELOCK_FILE, "Server can not lock: " + basename(filepath));
    }*/

    // opened, locked => OK
    send(client_sock, &ecode, 1, 0);

    long total_received = 0;
    do {
      if((recv_len = recv(client_sock, buffer, BUFFER_SIZE, 0)) == -1) {
        ecode = STATUS_CODE_EFILE_CONTENT;
        send(client_sock, &ecode, 1, 0);
        serverError(params, node_index, client_sock, STATUS_CODE_EFILE_CONTENT, "Error during data of file transmission");
        pthread_exit(NULL);
      }
      //cout<<"[CLIENT #"<<node_index<<"]"<<output_file.gcount()<<" B received. Total number of received bytes: "<<total_received<<" B / "<<filepath_len<<" B"<<endl;

      total_received += recv_len;

      output_file.write(buffer, recv_len);

      if(recv_len == 0) {
        if(file_size != total_received) {
          cout<<"[CLIENT #"<<node_index<<"] Transmition ended with error about file content size. Total number of received bytes: "<<total_received<<" B"<<endl;
          ecode = STATUS_CODE_EFILE_CONTENT;
          send(client_sock, &ecode, 1, 0);
          serverError(params, node_index, client_sock, STATUS_CODE_EFILE_CONTENT, "Server can not open: " + filepath);
          pthread_exit(NULL);
        } else {
          cout<<"[CLIENT #"<<node_index<<"] Transmition ended successfully. Total number of received bytes: "<<total_received<<" B"<<endl;
          send(client_sock, &ecode, 1, 0);
        }
      }
    } while (recv_len > 0);

    // close file
    output_file.close();

    // unlock file
    //flock(output_file.filedesc(), LOCK_UN);
  }
  // read
  else if (mode == READ) {
    cout<<"[CLIENT #"<<node_index<<"] wants read a file. Received filename: '"<<basename(filepath.c_str())<<"'"<<endl;

    // open file
    ifstream input_file;
    input_file.open(filepath.c_str(), fstream::in | fstream::binary);
    if(!input_file.is_open()) {
      ecode = STATUS_CODE_EOPEN_FILE;
      send(client_sock, &ecode, 1, 0);
      serverError(params, node_index, client_sock, STATUS_CODE_EOPEN_FILE, "Server can not open: " + filepath);
      pthread_exit(NULL);
    }

    // lock file
    /*if(flock(input_file.filedesc(), LOCK_SH | LOCK_NB) != 0) {
      ecode = ELOCK_FILE;
      send(client_sock, &ecode, 1, 0);
      serverError(params, node_index, client_sock, ELOCK_FILE, "Server can not lock: " + filepath);
      pthread_exit(NULL);
    }*/

    // opened, locked => OK
    send(client_sock, &ecode, 1, 0);

    long total_sent = 0;
    long file_size = 0;

    input_file.seekg(0, input_file.end);
    file_size = input_file.tellg();
    input_file.seekg(0, input_file.beg);

    while(input_file.read(buffer, BUFFER_SIZE)) {
      send(client_sock, buffer, BUFFER_SIZE, 0);
      total_sent += input_file.gcount();
      cout<<"[CLIENT #"<<node_index<<"] "<<input_file.gcount()<<" B sent. Total number of sent bytes: "<<total_sent<<" B / "<<file_size<<" B"<<endl;
    }

    send(client_sock, buffer, input_file.gcount(), 0);
    total_sent += input_file.gcount();
    cout<<"[CLIENT #"<<node_index<<"] Transmition ended. Total number of sent bytes: "<<total_sent<< " B / "<<file_size<<" B"<<endl;

    // close file
    input_file.close();

    // unlock file
    //flock(input_file.filedesc(), LOCK_UN);

  // else
  } else {
    ecode = STATUS_CODE_EHEADER;
    send(client_sock, &ecode, 1, 0);
    serverError(params, node_index, client_sock, STATUS_CODE_EHEADER, "Header error. Mode could not be recognized.");
    pthread_exit(NULL);
  }

  serverEnd(params, node_index, client_sock);
  pthread_exit(NULL);
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
    return params.ecode;
  }

  // addrinfo from given port
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  // get addrinfo
  if(getaddrinfo(NULL, params.port.c_str(), &hints, &results) != 0)
    error(EGETADDRINFO, "Host is not valid.\n");

  // create socket
  if((sock = socket(results->ai_family, results->ai_socktype, results->ai_protocol)) == -1)
    error(ESOCKET, "Socket can not be created.\n");

  // bind socket
  if(bind(sock, results->ai_addr, results->ai_addrlen) != 0)
    error(EBIND, "Bind failed.");

  // listen
  if(listen(sock, MAX_CLIENTS) < 0)
		error(ELISTEN, "Listen failed.");

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

    cout<<"[SERVER] Joined client. Started handling personal thread with number: "<<params.nodes_count<<"\n"<<endl;

    requests_count++;

    // create Tpthread_args
    Tpthread_args* threadarg = new Tpthread_args();
    threadarg->params = &params;
    threadarg->node_index = params.nodes_count;
    threadarg->addrinfo = results;
    threads_args[params.nodes_count] = threadarg;
    threadarg->sock = client_sock;

    // pthread per client
    if(pthread_create(&threads[params.nodes_count], NULL, handleServer, (void *) threadarg) != 0)
      error(ETHREAD, "Unable to create thread.\n");

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
