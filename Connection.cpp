#include "Connection.h"

#define DEBUG

using namespace std;

void Connection::parseArguments()
{
    // "http://"
    size_t position = http_path.find("http://");
    if (position == string::npos)
        cerr << "Error while parsing input arguments" << endl;

    // <server_hostname>
    size_t http_len = strlen("http://");
    size_t end_of_port_number = http_len;
    if (position == string::npos)
        cerr << "Error while parsing input arguments" << endl;
    else
    {
        end_of_port_number = http_path.find(":", http_len);
        if (end_of_port_number == string::npos)
        {
            end_of_port_number = http_path.find("/", http_len);
            port_number = 6677;     // implicit port number
        }
        server_hostname = http_path.substr(http_len, end_of_port_number - http_len);
    }

    // <port>
    size_t end_of_server_hostname = http_path.find("/", end_of_port_number);
    if (end_of_server_hostname != end_of_port_number)
        port_number = atoi((http_path.substr(end_of_port_number + 1, end_of_server_hostname - end_of_port_number - 1)).c_str());

    // <user>
    size_t end_of_username = http_path.find("/", end_of_port_number + 1);
    if (end_of_username == string::npos)
        cerr << "Error while parsing input arguments" << endl;
    else
        user = http_path.substr(end_of_port_number + 1, end_of_username - end_of_port_number - 1);

    // <remote_path>
    remote_path = http_path.substr(end_of_username + 1);

#ifdef DEBUG
    cout << "server_hostname: " << server_hostname << endl;
    cout << "port_number: " << port_number << endl;
    cout << "user: " << user << endl;
    cout << "remote_path: " << remote_path << endl;
#endif
}

void Connection::establishConnection()
{
    /*
     * ziskani adresy serveru pomoci DNS
     */

    if ((server = gethostbyname(server_hostname.c_str())) == NULL)
    {
        cerr << "ERROR: no such host as " << server_hostname << endl;
        exit(EXIT_FAILURE);
    }

    /*
     * nalezeni IP adresy serveru a inicializace struktury server_address
     */

    memset((char *) &server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    memcpy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port_number);

    /* Vytvoreni soketu */
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
    }
}

void Connection::communicate()
{
    /* nacteni zpravy od uzivatele */
    memset(buf, '\0', BUFSIZE);
    printf("Please enter msg: ");
    fgets(buf, BUFSIZE, stdin);

    if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0)
    {
        perror("ERROR: connect");
        exit(EXIT_FAILURE);
    }

    /* odeslani zpravy na server */
    bytestx = send(client_socket, buf, strlen(buf), 0);
    if (bytestx < 0)
        perror("ERROR in sendto");

    /* prijeti odpovedi a jeji vypsani */
    bytesrx = recv(client_socket, buf, BUFSIZE, 0);
    if (bytesrx < 0)
        perror("ERROR in recvfrom");

    printf("Echo from server: %s", buf);

    close(client_socket);
}


