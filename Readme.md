ASSESSMENT
=========
8.625/20b ([assessment report](https://github.com/ldrahnik/ipk_1_project_2/issues/1))

Klient-server pro jednoduchý přenos souborů
============

## Příklad spuštění:

```
./tests/server_root/ipk-server -h
Example of usage:

./ipk-server [-h] [-r <number>] -p <port> 

Options:
-h - show help message
-r <number> - number of handled requests, then server ends
-p <port> - specification port
```

```
./tests/client_root/ipk-client
Port is required.
Hostname is required.
Mode (write or read) is required.
File is required.

Example of usage:

./ipk-client -h <host> -p <port> [-r|-w] file

Options:
-h  -- show help message
-h <host> - hostname
-p <port> - specification port
[-r|-w] - file
```

## Omezení programu:

## Rozšíření programu:

Podpora IPv6.

## Testování programu:

```
make test
bash ./tests/tests.sh
*******TEST 1 PASSED
*******TEST 1.1 PASSED
*******TEST 2 PASSED
*******TEST 2.1 PASSED
```
