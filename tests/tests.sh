#!/bin/bash

# Name: Drahník Lukáš
# Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
# Date: 18.6.2020
# Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>
# File: tests.sh

CLIENT_ROOT=./tests/client_root
SERVER_ROOT=./tests/server_root
PORT=501261  
TEST_FILE_NAME=test.txt
TEST_FILE=./tests/$TEST_FILE_NAME
TEST_FILE_NAME_COPY=test.txt
TEST_FILE_COPY=./tests/$TEST_FILE_NAME

# 1 upload file IPv4
(cd $SERVER_ROOT && ./ipk-server -r 1 -p $PORT > /dev/null 2>&1 &) && (cd $CLIENT_ROOT && ./ipk-client -h localhost -p $PORT -w ../$TEST_FILE_NAME > /dev/null 2>&1);
if [ -f $SERVER_ROOT/$TEST_FILE_NAME ] && diff $SERVER_ROOT/$TEST_FILE_NAME $TEST_FILE > /dev/null; then
    echo "*******TEST 1 PASSED";
else
    echo "TEST 1 FAILED";
fi

# 1.1 upload file IPv6
(cd $SERVER_ROOT && ./ipk-server -r 1 -p $PORT > /dev/null 2>&1 &) && (cd $CLIENT_ROOT && ./ipk-client -h ::1 -p $PORT -w ../$TEST_FILE_NAME_COPY > /dev/null 2>&1);
if [ -f $SERVER_ROOT/$TEST_FILE_NAME_COPY ] && diff $SERVER_ROOT/$TEST_FILE_NAME_COPY $TEST_FILE_COPY > /dev/null; then
    echo "*******TEST 1.1 PASSED";
else
    echo "TEST 1.1 FAILED";
fi

# 2 download file IPv4
(cd $SERVER_ROOT && ./ipk-server -r 1 -p $PORT > /dev/null 2>&1 &) && (cd $CLIENT_ROOT && ./ipk-client -h localhost -p $PORT -r $TEST_FILE_NAME > /dev/null 2>&1);
if [ -f $CLIENT_ROOT/$TEST_FILE_NAME ] && diff $CLIENT_ROOT/$TEST_FILE_NAME $TEST_FILE > /dev/null; then
    echo "*******TEST 2 PASSED";
else
    echo "TEST 2 FAILED";
fi

# 2 clean
rm -f $CLIENT_ROOT/$TEST_FILE_NAME;

# 2.1 download file IPv6
(cd $SERVER_ROOT && ./ipk-server -r 1 -p $PORT > /dev/null 2>&1 &) && (cd $CLIENT_ROOT && ./ipk-client -h ::1 -p $PORT -r $TEST_FILE_NAME_COPY > /dev/null 2>&1);
if [ -f $CLIENT_ROOT/$TEST_FILE_NAME_COPY ] && diff $CLIENT_ROOT/$TEST_FILE_NAME_COPY $TEST_FILE_COPY > /dev/null; then
    echo "*******TEST 2.1 PASSED";
else
    echo "TEST 2.1 FAILED";
fi

# 2.1 clean
rm -f $CLIENT_ROOT/$TEST_FILE_NAME_COPY;

# 3 download file (threads - multiple clients at the same time)
(cd $SERVER_ROOT && ./ipk-server -r 2 -p $PORT > /dev/null 2>&1 &) && (cd $CLIENT_ROOT && ./ipk-client -h localhost -p $PORT -r $TEST_FILE_NAME_COPY > /dev/null 2>&1) && (cd $CLIENT_ROOT && ./ipk-client -h localhost -p $PORT -r $TEST_FILE_NAME_COPY > /dev/null 2>&1);
if [ -f $CLIENT_ROOT/$TEST_FILE_NAME ] && [ -f $CLIENT_ROOT/$TEST_FILE_NAME_COPY ] && diff $CLIENT_ROOT/$TEST_FILE_NAME $TEST_FILE > /dev/null && diff $CLIENT_ROOT/$TEST_FILE_NAME_COPY $TEST_FILE_COPY > /dev/null; then
    echo "*******TEST 3 PASSED";
else
    echo "TEST 3 FAILED";
fi

# final clean
rm -f $SERVER_ROOT/$TEST_FILE_NAME
rm -f $SERVER_ROOT/$TEST_FILE_NAME_COPY;
