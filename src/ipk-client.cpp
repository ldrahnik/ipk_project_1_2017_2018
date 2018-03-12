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

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

using namespace std;

#define OK 0
#define DOES_NOT_EXIST 1
#define CANNOT_WRITE 2
#define HEADER_ERROR 3
#define ALREADY_USED 4

#define BUFFER_SIZE 1448

#define MAX_CLIENTS 128

/**
 * When is pressed ctrl+c.
 */
static int G_break = 0;

/**
 * Help message.
 */
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
 * Error codes.
 */
enum ecodes {
  EOK = 0,               // ok
  EOPT = 1,              // invalid option (option argument is missing,
                         // unknown option, unknown option character)
  EGETADDRINFO = 2,
  ESOCKET = 3,
  EBIND = 4,
  ELISTEN = 5,
  EFILE = 6
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
  string port;                        // option p
  string host;                        // option h
  string filepath;                    // option [r|w]
  int mode;                           // option [r|w] r = 0, w = 1
  int ecode;                          // error code
} TParams;

/**
 * Print error message.
 */
void error(int code, string msg) {
	cerr<<msg<<endl;
	exit(code);
}

/**
 * Clean mess when is program closing successfuly or with error.
 *
 * @return void
 */
void clean() {

}

/**
 * Get TParams structure from terminal options, option arguments and nodes.
 *
 * @return TParams
 */
TParams getParams(int argc, char *argv[]) {

  // default params
  TParams params = { };
  params.ecode = EOK;
  params.mode = -1;

  // don't want getopt() writing to stderr
  opterr = 0;

  // getopt
  int c;
  while ((c = getopt(argc, argv, "h:p:r:w:")) != -1) {
    switch (c) {
      case 'h':
        params.host = optarg;
        break;
      case 'p':
        params.port = optarg;
        break;
      case 'r':
        params.mode = READ;
        params.filepath = optarg;
        break;
      case 'w':
        params.mode = WRITE;
        params.filepath = optarg;
        break;
      case '?':
        if(optopt == 'p' || optopt == 'h' || optopt == 'r' || optopt == 'w') {
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        } else if(isprint (optopt)) {
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        } else {
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        }
        params.ecode = EOPT;
        break;
      default:
        params.ecode = EOPT;
      }
  }

  if(params.port.empty())
		error(1, "Port is required.");
	if(params.host.empty())
		error(1, "Hostname is required.");
  if(params.mode == -1)
    error(1, "Mode (write or read) is required");
  if(params.mode == WRITE && params.filepath.empty())
    error(1, "Write file is required.");

  return params;
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
  host_info.ai_family = AF_INET;
  host_info.ai_socktype = SOCK_STREAM;
  
  // parsing parameters
  TParams params = getParams(argc, argv);
  if(params.ecode != EOK) {
    cout<<HELP_MSG<<endl;
    return params.ecode;
  }

  // try get file
  fstream file;
  if(params.mode == WRITE && !params.filepath.empty()) {
    file.open(params.filepath.c_str(), fstream::in | fstream::binary);
    if(!file.is_open())
      error(EFILE, "Error opening file to write on server: " + params.filepath);
  }

  // try get addrinfo
  if((getaddrinfo(params.host.c_str(), params.port.c_str(), &host_info, &host_ips)) != 0)
		error(EGETADDRINFO, "Hostname address is not valid.");

	// create socket, connect on given addres
	for (rp = host_ips; rp != NULL; rp = rp->ai_next) {
    if((sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1)
      continue;
		if(connect(sock, rp->ai_addr, rp->ai_addrlen) != -1)
			break;
		else
			close(sock);
	}

  // write = upload
  if(params.mode == WRITE) {
    char buffer[BUFFER_SIZE];

		int len = params.filepath.length();

    buffer[0] = params.mode;
    memcpy(buffer+1, &len, sizeof(int));
    params.filepath.copy(buffer+1+sizeof(int), len);

    send(sock, buffer, 1 + sizeof(int)+len, 0);

    // cekani na status report
    int recv_len;
    char status = 99;

    recv_len = recv(sock, &status, 1, 0);

    if (recv_len != 1)
      error(6, "chyba prijmu odpovedi na hlavicku");
    if (status == CANNOT_WRITE)
      error(10, "do pozadovaneho souboru nelze zapisovat!");
    if (status == HEADER_ERROR)
      error(6, "chyba odeslane hlavicky");
    if (status != OK)
      error(99, "neznama chyba serveru");

    long celkem = 0;
    long file_size = 0;

    file.seekg(0, file.end);   // nastaveni ukazatele na konec souboru
    file_size = file.tellg();  // ziskani pozice ukazatele (== velikost souboru)
    file.seekg(0, file.beg);   // nastaveni zpet na zacatek

    // posilani dat
    cout << "Posilam soubor: '" << file << "' Velikost: " << file_size << " B" << endl;
    while(file.read(buffer, BUFFER_SIZE)) { // dokud se z file nacte cela BUFFER_SIZE

      // CTRL+C handler
      if(G_break == 1)
        break;

      send(sock, buffer, BUFFER_SIZE, 0);
      celkem += file.gcount();
      cout << file.gcount() << " B odeslano. Celkem: " << celkem << " B z " << file_size << " B" << endl;
    }

    send(sock, buffer, file.gcount(), 0);
    celkem += file.gcount();
    cout << file.gcount() << " B odeslano. Celkem: " << celkem << " B z " << file_size << " B" << endl;

    file.close();
  } // download
	{
		char buffer[BUFFER_SIZE];

		int len = params.filepath.length();

		// posilani hlavicky
		buffer[0] = READ;
		memcpy(buffer+1, &len, sizeof(int));
		params.filepath.copy(buffer+1+sizeof(int), len);

		send(sock, buffer, 1+sizeof(int)+len, 0);

		// cekani na status report
		int recv_len;
		char status = 99;

		recv_len = recv(sock, &status, 1, 0);

		if (recv_len != 1)
			error(6, "chyba prijmu odpovedi na hlavicku");
		if (status == DOES_NOT_EXIST)
			error(10, "pozadovany soubor na serveru neexistuje!");
		if (status == HEADER_ERROR)
			error(6, "chyba odeslane hlavicky");
		if (status != OK)
			error(99, "neznama chyba serveru");

		// prijimani dat
		long celkem = 0;

		cout << "Prijimam soubor: '" << params.filepath << "'" << endl;
		do {
      // CTRL+C handler
      if(G_break == 1)
        break;

    recv_len = recv(sock, buffer, BUFFER_SIZE, 0);

    file.write(buffer, recv_len);

    if (recv_len == 0)
    {
    	cout << "Konec prenosu. Celkem preneseno: " << celkem << " B" << endl ;
    	break;
    }

    if (recv_len == -1)
    	cerr << "Chyba prenosu!" << endl ;

    celkem += recv_len;
    cout << recv_len << " B prijato. Celkem: " << celkem << " B" << endl ;
    } while (true);

		file.close();

	}

	close(sock);
  clean();

  return ecode;
}
