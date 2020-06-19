/**
 * Name: Lukáš Drahník
 * Project: IPK: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
 * Date: 19.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
 */

#include "ipk-server-params.h"

TParams getParams(int argc, char *argv[]) {

  // default params
  TParams params;
  params.ecode = EOK;
  params.show_help_message = 0;
  params.nodes_count = 0;
  params.requests_count = -1;

  // don't want getopt() writing to stderr
  opterr = 0;

  // getopt
  int c;
  while ((c = getopt(argc, argv, "hp:r:")) != -1) {
    switch (c) {
      case 'h':
        params.show_help_message = 1;
        return params;
      case 'p':
        params.port = optarg;
        break;
      case 'r':
        params.requests_count = atoi(optarg);
        break;
      case '?':
        if(optopt == 'p' || optopt == 'r')
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

  if(params.port.empty()) {
    fprintf(stderr, "Port is required.\n");
    params.ecode = EOPT;
  }

  return params;
}
