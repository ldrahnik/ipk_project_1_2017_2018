/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 19.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#include "ipk-client-params.h"

TParams getParams(int argc, char *argv[]) {

  // default params
  TParams params = { };
  params.ecode = EOK;
  params.transfer_mode = -1;

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
        params.transfer_mode = READ;
        params.filepath = optarg;
        break;
      case 'w':
        params.transfer_mode = WRITE;
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

  if(params.port.empty()) {
	fprintf(stderr, "Port is required.\n");
    params.ecode = EOPT;
  }
  if(params.host.empty()) {
	fprintf(stderr, "Hostname is required.\n");
    params.ecode = EOPT;
  } 
  if(params.transfer_mode == -1) {
    fprintf(stderr, "Mode (write or read) is required.\n");
    params.ecode = EOPT;
  }
  if(params.filepath.empty()) {
    fprintf(stderr, "File is required.\n");
    params.ecode = EOPT;
  }

  return params;
}
