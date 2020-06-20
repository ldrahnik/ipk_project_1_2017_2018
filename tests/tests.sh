#!/bin/bash

# Name: Drahník Lukáš
# Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
# Date: 18.6.2020
# Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
# File: tests.sh

TEST_DIRECTORY=`dirname $0`
CLIENT_ROOT=$TEST_DIRECTORY/client_root
SERVER_ROOT=$TEST_DIRECTORY/server_root
PORT=50126
TEST_FILE_NAME=test.txt
TEST_FILE=$TEST_DIRECTORY/$TEST_FILE_NAME

# 1 (upload file IPv4)
$SERVER_ROOT/ipk-server -r 1 -p $PORT > /dev/null & $CLIENT_ROOT/ipk-client -h localhost -p $PORT -w $TEST_FILE > /dev/null 2>&1
if [ -f $TEST_FILE_NAME ]; then
    echo "*******TEST 1 PASSED";
else
    echo "TEST 1 FAILED";
fi

# 1 úklid
rm -f $TEST_FILE_NAME

# 1.1 (upload file IPv6)
$SERVER_ROOT/ipk-server -r 1 -p $PORT > /dev/null & $CLIENT_ROOT/ipk-client -h ::1 -p $PORT -w $TEST_FILE > /dev/null 2>&1
if [ -f $TEST_FILE_NAME ]; then
    echo "*******TEST 1.1 PASSED";
else
    echo "TEST 1.1 FAILED";
fi

# 1.1 úklid
rm -f $TEST_FILE_NAME

# 2 příprava
cp $TEST_FILE ./

# 2 (download file)
$SERVER_ROOT/ipk-server -r 1 -p $PORT > /dev/null & $CLIENT_ROOT/ipk-client -h localhost -p $PORT -r $TEST_FILE_NAME > /dev/null 2>&1
if [ -f $TEST_FILE_NAME ]; then
    echo "*******TEST 2 PASSED";
else
    echo "TEST 2 FAILED";
fi

# 2 úklid
rm -f $TEST_FILE_NAME

# 2.1 příprava
cp $TEST_FILE ./

# 2.1 (download file IPv6)
$SERVER_ROOT/ipk-server -r 1 -p $PORT > /dev/null & $CLIENT_ROOT/ipk-client -h ::1 -p $PORT -r $TEST_FILE_NAME > /dev/null 2>&1
if [ -f $TEST_FILE_NAME ]; then
    echo "*******TEST 2.1 PASSED";
else
    echo "TEST 2.1 FAILED";
fi

# 2.1 úklid
rm -f $TEST_FILE_NAME
