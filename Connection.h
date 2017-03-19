#pragma once

#include <string>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
    #include <unistd.h>
#include <netdb.h>

using namespace std;

enum {
    OK = 200,
//    NOT_FOUND = 400,
//    BAD_REQUEST = 404
};

class Connection
{
    // buffer
    static const int BUFSIZE = 1024;
    char buf[BUFSIZE];

    //command line arguments
    string command;
    string http_path;

    // <command> http://<ip_address>:<port_number>/<user>/<remote_path>
    string server_hostname;
    int port_number;
    string user;
    string remote_path;

    int client_socket;

    struct hostent *server;
    struct sockaddr_in server_address;

    void parseArguments();
    void createHTTPHeader();
    void readResponse();
public:
    Connection(const string &command, const string &http_path)
            : command{command}, http_path{http_path} { parseArguments(); };

    void establishConnection();
    int communicate();

    string http_header;
};