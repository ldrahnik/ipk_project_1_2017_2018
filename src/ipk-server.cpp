/**
 * Name:							Lukáš Drahník
 * Project: 					IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date:							8.3.2018
 * Email:						  <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <math.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <sys/file.h>

#include <fstream>

using namespace std;

#define MAX_CLIENTS 128

#define BUFFER_SIZE 1448

/**
 * When is pressed ctrl+c.
 */
static int G_break = 0;

/**
 * Help message.
 */
const char *HELP_MSG = {
  "Example of usage:\n\n"
  "./ipk-server [-h] -p <port> \n\n"
  "Options:\n"
  "-h  -- show help message\n"
  "-p <port>  - specification port\n"
};

/**
 * Arguments for pthread created for each node.
 */
typedef struct pthread_args {
  struct params *params;
  int node_index;                     // start index is 0
  struct addrinfo *addrinfo;
  int sock;
} Tpthread_args;

/**
 * Print error message.
 */
void error(int code, string msg) {
	cerr<<msg<< endl;
  exit(code);
}

/**
 * Error codes.
 */
enum ecodes {
  EOK = 0,              // ok
  EOPT = 1,             // invalid option (option argument is missing,
                        // unknown option, unknown option character)
  EGETADDRINFO = 2,
  ESOCKET = 3,
  EBIND = 4,
  ELISTEN = 5,
  EFILE = 6,
  ETHREAD = 7,
  ETHREAD_CREATE = 8,

  /*#define OK 0
  #define DOES_NOT_EXIST 1
  #define CANNOT_WRITE 2*/
  EOPEN_FILE = 100,
  EHEADER = 101,
  ELOCK_FILE = 102
};

/**
 * Transfer file modes.
 */
enum modes {
  READ = 0,
  WRITE = 1
};

/**
 * Signal handler.
 */
void catchsignal(int sig) {
  if(sig == SIGINT) {
    G_break = 1;
  }
}

/**
 * Terminal parameters:
 */
typedef struct params {
  string port;
  int ecode;
  int nodes_count;
} TParams;

/**
 * Get TParams structure from terminal options, option arguments and nodes.
 *
 * @return TParams
 */
TParams getParams(int argc, char *argv[]) {

  // default params
  TParams params;
  params.ecode = EOK;
  params.nodes_count = 0;

  // don't want getopt() writing to stderr
  opterr = 0;

  // getopt
  int c;
  while ((c = getopt(argc, argv, "p:")) != -1) {
    switch (c) {
      case 'p':
        params.port = optarg;
        break;
      case '?':
        if(optopt == 'p')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if(isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        params.ecode = EOPT;
        break;
      default:
        params.ecode = EOPT;
    }
  }

  if(params.port.empty())
    error(1, "Port is required.");

  return params;
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
  //delete params->nodes;
}

void serverError(TParams* params, int node_index, int client_sock, int code, string msg) {
	cerr<<msg<<endl;
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
  char mode = buffer[0];
  int filepath_len;

  cout<<"[CLIENT #"<<node_index<<"] Is starting\n"<<endl;

  recv_len = recv(client_sock, buffer, 1+sizeof(int), 0);

  // header (size of file)
  if(recv_len != 1 + sizeof(int)) {
    ecode = EHEADER;
    send(client_sock, &ecode, 1, 0);
    serverError(params, node_index, client_sock, EHEADER, "Header error.");
    pthread_exit(NULL);
  }

  // size of file
  memcpy(&filepath_len, buffer+1, sizeof(int));

  // rest of header (filename)
  recv_len = recv(client_sock, buffer, filepath_len, 0);

  if(recv_len != filepath_len) {
    ecode = EHEADER;
    send(client_sock, &ecode, 1, 0);
    serverError(params, node_index, client_sock, EHEADER, "Header error.");
    pthread_exit(NULL);
  }

  // filename
  buffer[filepath_len] = '\0';
  string filepath = buffer;

  // write
  if(mode == WRITE) {

    // opening file failed
    ofstream output_file;
    output_file.open(basename(filepath.c_str()), fstream::out | fstream::binary);
    if(!output_file.is_open()) {
      ecode = EOPEN_FILE;
      send(client_sock, &ecode, 1, 0);
      serverError(params, node_index, client_sock, EOPEN_FILE, "Server can not open: " + filepath);
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

    cout<<"[CLIENT #"<<node_index<<"] Is receiving filename: '"<<basename(filepath.c_str())<<"'"<<endl;

    long total_received = 0;
    do {
      if((recv_len = recv(client_sock, buffer, BUFFER_SIZE, 0)) == -1) {
        cerr<<"WHAAAEE ERROR RECV!" << endl; // TODO:
      }
      //cout<<"[CLIENT #"<<node_index<<"]"<<output_file.gcount()<<" B received. Total number of received bytes: "<<total_received<<" B / "<<filepath_len<<" B"<<endl;

      total_received += recv_len;

      output_file.write(buffer, recv_len);

      if(recv_len == 0)
        cout<<"[CLIENT #"<<node_index<<"] Transmition ended. Total number of received bytes: "<<total_received<<" B"<<endl;

    } while (recv_len > 0);

    // close file
    output_file.close();

    // unlock file
    //flock(output_file.filedesc(), LOCK_UN);
  }
  // read
  else if (mode == READ) {

    // opening file failed
    ifstream input_file;
    input_file.open(filepath.c_str(), fstream::in | fstream::binary);
    if(!input_file.is_open()) {
      ecode = EOPEN_FILE;
      send(client_sock, &ecode, 1, 0);
      serverError(params, node_index, client_sock, EOPEN_FILE, "Server can not open: " + filepath);
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

    input_file.seekg(0, input_file.end);   // nastaveni ukazatele na konec souboru
    file_size = input_file.tellg();        // ziskani pozice ukazatele (== velikost souboru)
    input_file.seekg(0, input_file.beg);   // nastaveni zpet na zacatek

    cout<<"[CLIENT #"<<node_index<<"] Is sending filename: '"<<basename(filepath.c_str())<<"', Velikost: "<<file_size<<" B"<<endl;

    while(input_file.read(buffer, BUFFER_SIZE)) {
      send(client_sock, buffer, BUFFER_SIZE, 0);
      total_sent += input_file.gcount();
      cout<<"[CLIENT #"<<node_index<<"]"<<input_file.gcount()<<" B sent. Total number of sent bytes: "<<total_sent<<" B / "<<file_size<<" B"<<endl;
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
    ecode = EHEADER;
    send(client_sock, &ecode, 1, 0);
    serverError(params, node_index, client_sock, EHEADER, "Header error. Mode could not be recognized.");
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

  // get args
  TParams params = getParams(argc, argv);
  if(params.ecode != EOK) {
    cout<<HELP_MSG<<endl;
    return params.ecode;
  }

  // addrinfo from given port
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET6;
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

    // create client socket
    if((client_sock = accept(sock, (struct sockaddr *) &client_addr, &addr_size)) <= 0)
      continue;

    cout<<"[SERVER] Joined client. Started handling personal thread with number: "<<params.nodes_count<<"\n"<<endl;

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
  }

  // wait for all child node's
  for(int index = 0; index < params.nodes_count; index++)
    pthread_join(threads[index], NULL);

  // clean
  clean(&params, results, threads_args);

  return ecode;
}
