# Name:							Lukáš Drahník
# Project: 					Varianta 2: Klient-server pro jednoduchý přenos souborů (Veselý)
#	Date:							10.3.2018
# Email:						<xdrahn00@stud.fit.vutbr.cz>, <ldrahnik@gmail.com>

########################################### CLIENT

CLIENT_NAME     		= ipk-client
CLIENT_SOURCES  		= src/ipk-client.cpp
CLIENT_OBJECTS  		= $(CLIENT_SOURCES:.cpp=.o)
PROJECT_DOC					= doc/manual.pdf
PROJECT_README			= Readme.md

CC              		= g++
CFLAGS 							= -Wall -pedantic -pthread # -Wextra

############################################ SERVER

SERVER_NAME     		= ipk-server
SERVER_SOURCES  		= src/ipk-server.cpp
SERVER_OBJECTS  		= $(SERVER_SOURCES:.cpp=.o)

CC              		= g++
CFLAGS 							= -Wall -pedantic -pthread # -Wextra

############################################ MAKE CLEAN

all:			$(CLIENT_NAME) $(SERVER_NAME)

$(CLIENT_NAME):	$(CLIENT_OBJECTS)
		$(CC) $(CFLAGS) $(CLIENT_SOURCES) -o client_root/$@

$(SERVER_NAME):	$(SERVER_OBJECTS)
				$(CC) $(CFLAGS) $(SERVER_SOURCES) -o server_root/$@

clean:
	rm -rf *~ $(SERVER_OBJECTS) $(CLIENT_OBJECTS)
	cd doc && make clean

rebuild:	clean all

############################################ COMPRESS

LOGIN = xdrahn00
FILES = Makefile $(CLIENT_SOURCES) $(SERVER_SOURCES) $(PROJECT_DOC) $(PROJECT_README)

tar:
	tar -cvzf $(LOGIN).tar $(FILES)

rmtar:
	rm -f $(LOGIN).tar

############################################

tex:
	cd doc && make && make manual.ps && make manual.pdf
