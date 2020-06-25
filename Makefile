# Name:	Lukáš Drahník
# Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
# Date:	10.3.2018
# Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>

PROJECT_DOC				= doc/dokumentace.pdf
PROJECT_README			= Readme.md

########################################### PROTOCOL

PROTOCOL_SOURCES_CODE	= $(shell find src/ipk-protocol/ -name *.cpp)
PROTOCOL_SOURCES_HEADERS= src/ipk-protocol/*.h
PROTOCOL_SOURCES		= $(PROTOCOL_SOURCES_CODE) $(PROTOCOL_SOURCES_HEADERS)

########################################### CLIENT

CLIENT_CC				= g++
CLIENT_CFLAGS			= -Wall -Wextra -pedantic
CLIENT_NAME				= ipk-client
CLIENT_NAME_TESTS		= ipk-client-tests
CLIENT_SOURCES_CODE		= $(shell find src/ipk-client/ -name *.cpp)
CLIENT_SOURCES_HEADERS	= src/ipk-client/*.h
CLIENT_SOURCES			= $(CLIENT_SOURCES_CODE) $(CLIENT_SOURCES_HEADERS) $(PROTOCOL_SOURCES)
CLIENT_OBJECTS			= $(CLIENT_SOURCES_CODE:%.cpp=%.o)

############################################ SERVER

SERVER_CC				= g++
SERVER_CFLAGS			= -Wall -Wextra -pedantic -pthread
SERVER_NAME				= ipk-server
SERVER_NAME_TESTS		= ipk-server-tests
SERVER_SOURCES_CODE		= $(shell find src/ipk-server/ -name *.cpp)
SERVER_SOURCES_HEADERS	= src/ipk-server/*.h
SERVER_SOURCES			= $(SERVER_SOURCES_CODE) $(SERVER_SOURCES_HEADERS) $(PROTOCOL_SOURCES)
SERVER_OBJECTS			= $(SERVER_SOURCES_CODE:%.cpp=%.o)

############################################

all: $(CLIENT_NAME) $(CLIENT_NAME_TESTS) $(SERVER_NAME) $(SERVER_NAME_TESTS)

$(CLIENT_NAME): $(CLIENT_OBJECTS)
	$(CLIENT_CC) $(CLIENT_CFLAGS) $(CLIENT_SOURCES) -o $@

$(CLIENT_NAME_TESTS): $(CLIENT_OBJECTS)
	$(CLIENT_CC) $(CLIENT_CFLAGS) $(CLIENT_SOURCES) -o ./tests/client_root/$(CLIENT_NAME)

$(SERVER_NAME):	$(SERVER_OBJECTS)
	$(SERVER_CC) $(SERVER_CFLAGS) $(SERVER_SOURCES) -o $@

$(SERVER_NAME_TESTS): $(SERVER_OBJECTS)
	$(SERVER_CC) $(SERVER_CFLAGS) $(SERVER_SOURCES) -o ./tests/server_root/$(SERVER_NAME)

clean:
	rm -rf *~ $(SERVER_OBJECTS) $(CLIENT_OBJECTS)
	cd doc && make clean

rebuild: clean all

############################################ ARCHIVE

ARCHIVE_NAME = xdrahn00
ARCHIVE_FILES = Makefile $(CLIENT_SOURCES) $(SERVER_SOURCES) $(PROTOCOL_SOURCES) $(PROJECT_DOC) $(PROJECT_README)

zip:
	zip -r $(ARCHIVE_NAME).zip $(ARCHIVE_FILES)

rmzip:
	rm -f $(ARCHIVE_NAME).zip

############################################ LATEX

tex:
	cd doc && make && make dokumentace.ps && make dokumentace.pdf

############################################ TESTS

test:
	bash ./tests/tests.sh
