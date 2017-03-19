CC=g++
CPPFLAGS=-Wall -pedantic -std=c++11

all: client server

client: client.cpp Connection.cpp
	${CC} ${CPPFLAGS} client.cpp Connection.cpp -o ftrest

server: server.cpp Server.cpp
	${CC} ${CPPFLAGS} server.cpp Server.cpp -o ftrestd

clean:
	rm -vf *.o