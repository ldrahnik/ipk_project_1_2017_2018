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

#include <fstream>

using namespace std;

#define MAX_CLIENTS 128

#define BUFFER_SIZE 1448

#define OK 0
#define DOES_NOT_EXIST 1
#define CANNOT_WRITE 2
#define HEADER_ERROR 3
#define ALREADY_USED 4

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
  int sock;                           // in UDP case is used in both sides (sending, listening)
} Tpthread_args;

/**
 * Print error message.
 */
void error(int code, string msg) {
	cerr<<msg<< endl;
	exit(code);
}

/**
 * Node structure.
 */
typedef struct node {
  char* node;         // IPv4/IPv6/hostname adresa uzlu
  int ecode;          // error code
  float specific_rtt; // <uzel;RTT> default value is -1
} TNode;

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
  ETHREAD_CREATE = 8
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
  string port;                           // option p
  int ecode;                             // error code
  int nodes_count;
  struct node *nodes;
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
void clean() {

}


/**
 * @return void*
 */
void* handleServer(void *threadarg) {
  //TParams *params = (TParams *) threadarg;

  struct pthread_args *pthread_args = (struct pthread_args *) threadarg;
  //struct node node = (struct node) pthread_args->params->nodes[pthread_args->node_index];
  //struct addrinfo *addrinfo = pthread_args->addrinfo;
  //TParams* params = (TParams*) pthread_args->params;
  int client_socket = pthread_args->sock;
  int node_index = pthread_args->node_index;

  /*struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  addr_size = sizeof serverStorage;

  struct sockaddr_in6 server_addr;
  char buffer[IP_MAXPACKET];
  int sock;
  long recv_len;
  struct timeval tv;

  fd_set my_set;*/

  cout << "WHEEEE4\n" << endl;
  cout << node_index << endl;

  // prijem
  ssize_t recv_len;
  char buffer[BUFFER_SIZE];

  recv_len = recv(client_socket, buffer, 1+sizeof(int), 0);

  if (recv_len != 1+sizeof(int))
  {
    cerr << "chyba prijmu hlavicky" << endl;
    char status = HEADER_ERROR;
    send(client_socket, &status, 1, 1);
    cout << "child dying, pid: " << getpid() << endl;
    close(client_socket);
    exit(0);
  }

  char mode = buffer[0];
  int filepath_len;
  memcpy(&filepath_len, buffer+1, sizeof(int));

  // prijeti zbytku hlavicky (nazev souboru)
  recv_len = recv(client_socket, buffer, filepath_len, 0);

  if (recv_len != filepath_len)
  {
    cerr << "chyba prijmu hlavicky" << endl;
    char status = HEADER_ERROR;
    send(client_socket, &status, 1, 1);
    cout << "child dying, pid: " << getpid() << endl;
    close(client_socket);
    exit(0);
  }

  buffer[filepath_len] = '\0';

  string filepath = buffer;

  if (mode == WRITE)
  {
    ofstream output_file;

    output_file.open(filepath.c_str(), fstream::out | fstream::binary);

    if (!output_file.is_open())
    {
      cerr << "cannot write to: " + filepath << endl;
      char status = CANNOT_WRITE;
      send(client_socket, &status, 1, 0);
      cout << "child dying, pid: " << getpid() << endl;
      close(client_socket);
      exit(0);
    }
    else
    {
      char status = OK;
      // C++ knihovna na eve bohuzel nepodporuje filedesc()
      /*if (flock(output_file.filedesc(), LOCK_EX | LOCK_NB) != 0)  // zapis -> exkluzivni zamek
      {
        cerr << "soubor '" + filepath + "' nemohl byt zamcen" << endl;
        status = ALREADY_USED;
      }*/

      send(client_socket, &status, 1, 0);

      long celkem = 0;

      cout << "Prijimam soubor: '" << filepath << "'" << endl;
      do {
      recv_len = recv(client_socket, buffer, BUFFER_SIZE, 0);
      celkem += recv_len;

      output_file.write(buffer, recv_len);

      if (recv_len == 0)
        cout << "Konec prenosu. Celkem prijato: " << celkem << " B" << endl ;

      if (recv_len == -1)
        cerr << "Chyba prenosu!" << endl ;

      } while (recv_len > 0);

      //flock(output_file.filedesc(), LOCK_UN);
      output_file.close();
    }
  }
  else if (mode == READ)
  {
    ifstream input_file;

    input_file.open(filepath.c_str(), fstream::in | fstream::binary);

    if (!input_file.is_open())
    {
      cerr << filepath + "does not exist" << endl;
      char status = DOES_NOT_EXIST;
      send(client_socket, &status, 1, 0);
      cout << "child dying, pid: " << getpid() << endl;
      close(client_socket);
      exit(0);
    }
    else
    {
      char status = OK;
      /*if (flock(input_file.filedesc(), LOCK_SH | LOCK_NB) != 0)  // cteni -> sdileny zamek (ale nedovoli zapsat)
      {
        cerr << "soubor '" + filepath + "' nemohl byt zamcen" << endl;
        status = ALREADY_USED;
      }*/

      send(client_socket, &status, 1, 0);

      long celkem = 0;
      long file_size = 0;

      input_file.seekg(0, input_file.end);   // nastaveni ukazatele na konec souboru
      file_size = input_file.tellg();        // ziskani pozice ukazatele (== velikost souboru)
      input_file.seekg(0, input_file.beg);   // nastaveni zpet na zacatek

      cout << "Posilam soubor: '" << filepath << "' Velikost: " << file_size << " B" << endl;
      while(input_file.read(buffer, BUFFER_SIZE))
      {
        send(client_socket, buffer, BUFFER_SIZE, 0);
        celkem += input_file.gcount();
        //cout << input_file.gcount() << " B odeslano. Celkem: " << celkem << " B z " << file_size << " B" << endl;
      }

      send(client_socket, buffer, input_file.gcount(), 0);
      celkem += input_file.gcount();
      cout << "Celkem odeslano: " << celkem << " B z " << file_size << " B" << endl;

      //flock(input_file.filedesc(), LOCK_UN);
      input_file.close();
    }
  }
  else
  {
    char status = HEADER_ERROR;
    send(client_socket, &status, 1, 1);
  }

  cout << "child dying, pid: " << getpid() << endl;
  close(client_socket);
  exit(0);

  // install signal handler for CTRL+C
  /*signal(SIGINT, catchsignal);

  int index;
  pthread_t threads[params.nodes_count];
  Tpthread_args* threads_args[params.nodes_count];

  while(true) {

    // CTRL+C handler
    if(G_break == 1) {
      break;
    }

    // create Tpthread_args
    Tpthread_args* threadarg = new Tpthread_args();
    threadarg->params = &params;
    threadarg->node_index = index;
    threadarg->addrinfo = results;
    threads_args[index] = threadarg;

    int sock;
    if((sock = socket(PF_INET6, SOCK_DGRAM, 0)) == -1) {
      fprintf(stderr, "Socket can not be created.\n");
      ecode = ESOCKET;
      break;
    }
    threadarg->sock = sock;

    if(pthread_create(&threads[index], NULL, handleSever, (void *) threadarg) != 0) {
      fprintf(stderr, "Error: unable to create thread %i.\n", index);
      ecode = ETHREAD;
      break;
    }
  }*/

  // create socket
  /*if((sock = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "Socket can not be created. Run program as sudo.\n");

    pthread_exit(NULL);
  }

  memset((char *)&server_addr, 0, sizeof(server_addr));
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_addr = in6addr_any;
  server_addr.sin6_port = htons(params->port);

  int no = 0;
  if(setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no))) {
    fprintf(stderr, "setsockopt() for IPV6_V6ONLY error.\n");

    // close sock
    close(sock);

    pthread_exit(NULL);
  }

  // install signal handler for CTRL+C
  signal(SIGINT, catchsignal);

  if(bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    fprintf(stderr, "Socket can not be binded. Port is probably already in use.\n");

    // close sock
    close(sock);

    pthread_exit(NULL);
  }

  while(true) {

    // CTRL+C handler
    if(G_break == 1) {
      break;
    }

    FD_ZERO(&my_set);
    FD_SET(sock, &my_set);
    if(select(sock + 1, &my_set, NULL, NULL, &tv) < 0) {
      fprintf(stderr, "Select failed.\n");
      break;
    }
    if(FD_ISSET(sock, &my_set)) {

      if((recv_len = recvfrom(sock, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&serverStorage, &addr_size)) == -1) {
        fprintf(stderr, "Error during recvfrom: %s\n", strerror(errno));

        // close sock
        close(sock);

        pthread_exit(NULL);
      }

      if(sendto(sock, buffer, recv_len, 0, (struct sockaddr *)&serverStorage, addr_size) == -1) {
        fprintf(stderr, "Error during sendto: %s\n", strerror(errno));

        // close sock
        close(sock);

        pthread_exit(NULL);
      }
    }
  }*/

  // close
  //close(sock);

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
  int sock;
  struct addrinfo* results;
  struct addrinfo hints;
  //pthread_t thread;

  // parsing parameters
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

  if(getaddrinfo(NULL, params.port.c_str(), &hints, &results) != 0) {
    error(1, "Host is not valid.\n");
    return EGETADDRINFO;
  }

  // create socket
  if((sock = socket(results->ai_family, results->ai_socktype, results->ai_protocol)) == -1) {
    error(1, "Socket can not be created.\n");
    return ESOCKET;
  }

  fprintf(stderr, "WHEEE \n");

  // bind on socket
  if(bind(sock, results->ai_addr, results->ai_addrlen) != 0) {
    error(1, "Bind failed.");
    return EBIND;
  }

  fprintf(stderr, "WHEEE2 \n");

  if(listen(sock, MAX_CLIENTS) < 0) {
		error(1, "Listen failed.");
    return ELISTEN;
  }

  fprintf(stderr, "WHEEE3 \n");

  cout << "DEBUG: Server bezi, port: " << params.port << "\n" << endl;

  pthread_t threads[MAX_CLIENTS];
  Tpthread_args* threads_args[MAX_CLIENTS];

  while(true) {

    // CTRL+C handler
    if(G_break == 1)
      break;

    // create Tpthread_args
    Tpthread_args* threadarg = new Tpthread_args();
    threadarg->params = &params;
    threadarg->node_index = params.nodes_count;
    threadarg->addrinfo = results;

    threads_args[params.nodes_count] = threadarg;

    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);

    int client_socket = accept(sock, (struct sockaddr *) &client_addr, &addr_size);

    if(client_socket <= 0)
      continue;

    threadarg->sock = client_socket;

    cout<< params.nodes_count << endl;

    params.nodes_count++;

    if(pthread_create(&threads[params.nodes_count], NULL, handleServer, (void *) threadarg) != 0) {
      error(1, "Unable to create thread.\n");
      ecode = ETHREAD;
      break;
    }
  }

  // wait for all child node's
  for(int index = 0; index < params.nodes_count; index++) {
    pthread_join(threads[index], NULL);
  }

  // clean
  freeaddrinfo(results);

  for (int index = 0; index < params.nodes_count; index++) {

    // close sock
    close(threads_args[index]->sock);

    // free node addrinfo struct
    freeaddrinfo(threads_args[index]->addrinfo);

    // delete ThreadArgs[] (created with new)
    delete threads_args[index];
  }

  // delete TNode[] (created with new)
  delete params.nodes;

  clean();

  return ecode;
}
