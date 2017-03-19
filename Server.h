#pragma once

#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <sstream>
#include <sys/stat.h>

using namespace std;

class Server {
//    int rc;
    int welcome_socket;
    struct sockaddr_in6 sa;
    struct sockaddr_in6 sa_client;
    socklen_t sa_client_len;

    // Data from client
    string method;
    string user;
    string local_path;
    string type;
    void parseRequest(char *buff);

    // Data for client
    string httpHeader;
    string content;
    void createResponseHeader(string response_code);
    void doCommand();

public:
    string root_folder = ".";
    int port_number = 6677;

    void establishConnection();
    void doResponse();
};

