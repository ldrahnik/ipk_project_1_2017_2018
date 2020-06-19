# Name:	Lukáš Drahník
# Project: Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
# Date:	10.3.2018
# Email: <xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>

PROJECT_DOC				= doc/dokumentace.pdf
PROJECT_README			= Readme.md

########################################### CLIENT

CLIENT_NAME     		= ipk-client
CLIENT_SOURCES  		= src/ipk-client/*.cpp src/ipk-client/*.h
CLIENT_OBJECTS  		= $(CLIENT_SOURCES:.cpp=.o)

############################################ SERVER

SERVER_NAME     		= ipk-server
SERVER_SOURCES  		= src/ipk-server/*.cpp src/ipk-server/*.h
SERVER_OBJECTS  		= $(SERVER_SOURCES:.cpp=.o)

############################################

CC              		= g++
CFLAGS 					= -Wall -Wextra -pedantic -pthread

all: $(CLIENT_NAME) $(SERVER_NAME)

$(CLIENT_NAME):	$(CLIENT_OBJECTS)
		$(CC) $(CFLAGS) $(CLIENT_SOURCES) -o $@ -o ./tests/client_root/$@

$(SERVER_NAME):	$(SERVER_OBJECTS)
				$(CC) $(CFLAGS) $(SERVER_SOURCES) -o $@ -o ./tests/server_root/$@

clean:
	rm -rf *~ $(SERVER_OBJECTS) $(CLIENT_OBJECTS)
	cd doc && make clean

rebuild: clean all

############################################ ARCHIVE

ARCHIVE_NAME = xdrahn00
ARCHIVE_FILES = Makefile $(CLIENT_SOURCES) $(SERVER_SOURCES) $(PROJECT_DOC) $(PROJECT_README)

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
