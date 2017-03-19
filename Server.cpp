#include "Server.h"

void Server::establishConnection()
{
    sa_client_len = sizeof(sa_client);
    if ((welcome_socket = socket(PF_INET6, SOCK_STREAM, 0)) < 0)
    {
        perror("ERROR: socket");
        delete this;
        exit(EXIT_FAILURE);
    }

    memset(&sa,0,sizeof(sa));
    sa.sin6_family = AF_INET6;
    sa.sin6_addr = in6addr_any;
    sa.sin6_port = htons(port_number);

    if (bind(welcome_socket, (struct sockaddr*)&sa, sizeof(sa)) < 0)
    {
        perror("ERROR: bind");
        delete this;
        exit(EXIT_FAILURE);
    }

    if ((listen(welcome_socket, 1)) < 0)
    {
        perror("ERROR: listen");
        delete this;
        exit(EXIT_FAILURE);
    }
}

void Server::doResponse()
{
    int comm_socket;

    while(1)
    {
        comm_socket = accept(welcome_socket, (struct sockaddr*)&sa_client, &sa_client_len);
        if (comm_socket > 0)
        {
            char buff[1024];
            for (;;)
            {
                if (recv(comm_socket, buff, 1024, 0) <= 0)
                    break;

                parseRequest(buff);
                memset(buff, '\0', 1024);
                doCommand();
                strcpy(buff, httpHeader.c_str());

                send(comm_socket, buff, strlen(buff), 0);
            }
        }
        else
            printf(".");

        close(comm_socket);
    }
}

void Server::parseRequest(char *buff)
{
    string message(buff);
    istringstream stream(message);

    // one line from HTTP header
    string line ;

    getline(stream, line);

    // METHOD
    if ( line.find("GET") != string::npos )
        method = "GET";
    else if ( line.find("PUT") != string::npos)
        method = "PUT";
    else if (line.find("DELETE") != string::npos)
        method = "DELETE";

    // slash after user
    size_t pos = line.find_first_of("/", method.length() + 2);

    // USER
    user = line.substr(method.length() + 2, pos - method.length() - 2);

    // first "?" on first line
    size_t delimiter = line.find_first_of("?", pos);

    // PATH
    local_path = line.substr(pos + 1, delimiter - pos - 1);

    // type=file|folder
    size_t type_pos = line.find("type=", delimiter);
    size_t end_of_line = line.find("HTTP/1.1", type_pos);

    // TYPE
    type = line.substr(type_pos + strlen("type="), end_of_line - strlen("type=") - 1 - type_pos);
}

void Server::doCommand()
{
    DIR *dir;
    struct dirent *ent;
    struct stat s;

    // default response code if everything went good
    string response_code = "200 OK";

    // reset content of string to be sent to user
    content = "";

    // first we try if user folder exist on server
    if ((dir = opendir(user.c_str())) != NULL)
        closedir(dir);
    else
    {
        response_code = "404 Not Found";
        content = "User Account Not Found.\n";
    }

    // then we process request sent from client
    if (method == "GET" && type == "folder" && response_code == "200")
    {
        if (stat((/*root_folder + "/" +*/ user + "/" + local_path).c_str(), &s) == 0) {
            if (s.st_mode & S_IFDIR)
            {
                if ((dir = opendir((/*root_folder + "/" +*/ user + "/" + local_path).c_str())) != NULL)
                {
                    /* print all the files and directories within directory */
                    while ((ent = readdir(dir)) != NULL)
                    {
                        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                            continue;

                        content += ent->d_name;
                        content += "\t";
                    }
                    closedir(dir);
                }
            } else {
                response_code = "400 Bad Request";
                content = "Not a directory.\n";
            }
        } else {
            response_code = "404 Not Found";
            content = "Directory not found.\n";
        }
    }
    else if (method == "DELETE" && type == "folder" && response_code == "200")
    {
        if (stat((/*root_folder + "/" +*/ user + "/" + local_path).c_str(), &s) == 0) {
            if (s.st_mode & S_IFDIR)
                remove((/*root_folder + "/" +*/ user + "/" + local_path).c_str());
            else {
                response_code = "400 Bad Request";
                content = "Not a directory.\n";
            }
        } else {
            response_code = "404 Not Found";
            content = "Directory not found.\n";
        }
    }
    else if (method == "DELETE" && type == "file" && response_code == "200")
    {
        if (stat((/*root_folder + "/" +*/ user + "/" + local_path).c_str(), &s) == 0) {
            if (s.st_mode & S_IFREG)
                remove((/*root_folder + "/" + */user + "/" + local_path).c_str());
            else {
                response_code = "400 Bad Request";
                content = "Not a file.\n";
            }
        } else {
            response_code = "404 Not Found";
            content = "File not found.\n";
        }
    }
    else if (method == "PUT" && type == "folder" && response_code == "200") {
        if (mkdir((/*root_folder + "/" +*/ user + "/" + local_path).c_str(), 0755) < 0)
        {
            response_code = "404 Not Found";
            content = "Already exists.\n";
        }
    }
    else if (method == "GET" && type == "file" && response_code == "200") {
        if ((dir = opendir((/*root_folder + "/" +*/ user + "/" + local_path).c_str())) == NULL)
        {
            response_code = "404 Not Found";
            content = "File not found.\n";
        } else
            closedir(dir);
    }
    else if (method == "PUT" && type == "file" && response_code == "200") {
        if ((dir = opendir((/*root_folder + "/" +*/ user + "/" + local_path).c_str())) == NULL)
        {
            response_code = "404 Not Found";
            content = "Already exists.\n";
        } else
            closedir(dir);
    }

    createResponseHeader(response_code);
}

void Server::createResponseHeader(string response_code)
{
    httpHeader = "";

    httpHeader += "HTTP/1.1 " + response_code;

    //  Date  - Timestamp serveru v době vyřízení požadavku.
    httpHeader += "\r\nDate: ";
    char date[255];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    httpHeader += date;

    // Content-Type - typ obsahu odpovědi podle MIME
    httpHeader += "\r\nContent-Type: text/plain";

    // Content-Length - délka obsahu odpovědi
    httpHeader += "\r\nContent-Length: " + to_string(content.length());

    // Content-Encoding - typ kódování obsahu (identity, gzip, deflate)
    httpHeader += "\r\nContent-Encoding: identity\r\n\r\n";

    // Content
    httpHeader += content;
}
