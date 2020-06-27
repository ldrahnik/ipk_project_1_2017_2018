ASSESSMENT
=========
8.625/20b ([assessment report](https://github.com/ldrahnik/ipk_1_project_2/issues/1))

Klient-server pro jednoduchý přenos souborů
============

## Příklad spuštění:

```
./ipk-server -h
Example of usage:

./ipk-server [-h] [-r <number>] -p <port> 

Options:
-h - show help message
-r <number> - number of handled requests, then server ends
-p <port> - specification port
```

```
./ipk-client
Hostname is required.
Port is required.
Mode (write or read) is required.
File is required.

Example of usage:

./ipk-client -h <host> -p <number> -r|-w <file>

Options:
-h <host> - hostname
-p <number> - port
-r|-w <file> - read/write file
```

## Omezení programu:

## Rozšíření programu:

Přidaný volitelný parametr udávající konečný počet requestů, které server obslouží `[-r <number>]`. Defaultně neomezeně.

Podpora IPv6.

Server využívá vlákna a podporuje tak přístup více klientů v jednu chvíli.

## Testování programu:

```
make test
bash ./tests/tests.sh
*******TEST 1 PASSED
*******TEST 1.1 PASSED
*******TEST 2 PASSED
*******TEST 2.1 PASSED
*******TEST 3 PASSED
```

## Odevzdané soubory:

```
xdrahn00
├── doc
│   └── dokumentace.pdf
├── Makefile
├── Readme.md
├── src
│   ├── ipk-client
│   │   ├── ipk-client.cpp
│   │   ├── ipk-client-error.cpp
│   │   ├── ipk-client-error.h
│   │   ├── ipk-client.h
│   │   ├── ipk-client-params.cpp
│   │   └── ipk-client-params.h
│   ├── ipk-file-transfer
│   │   ├── ipk-file-transfer.cpp
│   │   ├── ipk-file-transfer-error.cpp
│   │   ├── ipk-file-transfer-error.h
│   │   └── ipk-file-transfer.h
│   ├── ipk-protocol
│   │   ├── ipk-protocol.cpp
│   │   └── ipk-protocol.h
│   └── ipk-server
│       ├── ipk-server.cpp
│       ├── ipk-server-error.cpp
│       ├── ipk-server-error.h
│       ├── ipk-server.h
│       ├── ipk-server-params.cpp
│       └── ipk-server-params.h
└── tests
    ├── client_root
    ├── server_root
    ├── test (copy).txt
    ├── tests.sh
    └── test.txt

9 directories, 24 files
```
