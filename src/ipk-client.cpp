/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 8.3.2018
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
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
  EOK = 0,               // ok, even used in protocol error
  EOPT = 1,              // invalid option (option argument is missing,
                         // unknown option, unknown option character)
  EGETADDRINFO = 2,
  ESOCKET = 3,
  EBIND = 4,
  ELISTEN = 5,
  EFILE = 6,


  // protocol error codes
  EOPEN_FILE = 100,
  EHEADER = 101,
  ELOCK_FILE = 102,
  EFILE_CONTENT = 103,
  EUNKNOWN = 99
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
		error(EOPT, "Port is required.");
	if(params.host.empty())
		error(EOPT, "Hostname is required.");
  if(params.mode == -1)
    error(EOPT, "Mode (write or read) is required");
  if(params.mode == WRITE && params.filepath.empty())
    error(EOPT, "Write file is required.");

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
  ssize_t recv_len;

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
  if(params.mode == READ && !params.filepath.empty()) {
    file.open(params.filepath.c_str(), fstream::out | fstream::binary | fstream::trunc);
    if(!file.is_open())
      error(EOPEN_FILE, "Error opening file to write on client: " + params.filepath);
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

  // write
  if(params.mode == WRITE) {
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
    char response = EUNKNOWN;

    // waiting on response on header
		if((recv(sock, &response, 1, 0)) != 1)
      error(EHEADER, "Header was not succesfully transfered.");

    // header response
    switch(response) {
      case EOPEN_FILE:
        error(EOPEN_FILE, "File can not be opened.");
        break;
      case ELOCK_FILE:
        error(ELOCK_FILE, "File can not be locked.");
        break;
      case EHEADER:
        error(EHEADER, "Header error.");
        break;
      case EOK:
        break;
      case EUNKNOWN:
        error(EUNKNOWN, "Unknown response.");
        break;
      default:
        error(EUNKNOWN, "Unknown response.");
        break;
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
		char response = EUNKNOWN;

    // header
		buffer[0] = READ;
		memcpy(buffer+1, &len, sizeof(int));
		params.filepath.copy(buffer+1+sizeof(int), len);

    // send header
		send(sock, buffer, 1+sizeof(int)+len, 0);

    // waiting on response on header
		if((recv(sock, &response, 1, 0)) != 1)
      error(EHEADER, "Header was not succesfully transfered.");

    // header response
    switch(response) {
      case EOPEN_FILE:
        error(EOPEN_FILE, "File can not be opened.");
        break;
      case ELOCK_FILE:
        error(ELOCK_FILE, "File can not be locked.");
        break;
      case EHEADER:
        error(EHEADER, "Header error.");
        break;
      case EOK:
        break;
      default:
        error(EUNKNOWN, "Unknown response.");
        break;
    }

		// data receiving
		long total_received = 0;

		cout<<"Receiving file: '"<<params.filepath<<"'"<<endl;

		do {
      if((recv_len = recv(sock, buffer, BUFFER_SIZE, 0)) == -1) {
        error(EFILE_CONTENT, "Transmission content");
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
