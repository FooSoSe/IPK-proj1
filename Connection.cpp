#include "Connection.h"

//#define DEBUG

using namespace std;

void Connection::parseArguments()
{
    /*
    del smaže soubor určený REMOTE-PATH na serveru
    get zkopíruje soubor z REMOTE-PATH do aktuálního lokálního adresáře či na místo určené pomocí LOCAL-PATH je-li uvedeno
    put zkopíruje soubor z LOCAL-PATH do adresáře REMOTE-PATH
    lst  vypíše obsah vzdáleného adresáře na standardní výstup (formát bude stejný jako výstup z příkazu ls)
    mkd vytvoří adresář specifikovaný v REMOTE-PATH na serveru
    rmd odstraní adresář specifikovaný V REMOTE-PATH ze serveru
     */
    if (command != "del" && command != "get" && command != "put" &&
        command != "lst" && command != "mkd" && command != "rmd" )
    {
        cerr << "Bad command given on command line" << endl;
        exit(EXIT_FAILURE);
    }

    // "http://"
    size_t position = http_path.find("http://");
    if (position == string::npos)
    {
        cerr << "Error while reading request given on command line\n" << endl;
        exit(EXIT_FAILURE);
    }

    // <server_hostname>
    size_t http_len = strlen("http://");
    size_t end_of_server_hostname = http_path.find(":", http_len);
    if (end_of_server_hostname == string::npos)
    {
        end_of_server_hostname = http_path.find("/", http_len);
        port_number = 6677;     // implicit port number
    }
    server_hostname = http_path.substr(http_len, end_of_server_hostname - http_len);

    // <port>
    size_t end_of_port_number = http_path.find("/", end_of_server_hostname);
    if (end_of_server_hostname != end_of_port_number)
        port_number = atoi((http_path.substr(end_of_server_hostname + 1, end_of_port_number - end_of_server_hostname - 1)).c_str());

    // <user>
    size_t end_of_username = http_path.find("/", end_of_port_number + 1);
    if (end_of_username == string::npos)
    {
        cerr << "Error while reading request given on command line\n" << endl;
        exit(EXIT_FAILURE);
    }
    else
        user = http_path.substr(end_of_port_number + 1, end_of_username - end_of_port_number - 1);

    // <remote_path>
    remote_path = http_path.substr(end_of_username + 1);

#ifdef DEBUG
    cout << "======input======" << endl;
    cout << "server_hostname: " << server_hostname << endl;
    cout << "port_number: " << port_number << endl;
    cout << "user: " << user << endl;
    cout << "remote_path: " << remote_path << endl;
    cout << "=================" << endl;
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
    memcpy(server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port_number);

    /* Vytvoreni soketu */
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
    }
}

int Connection::communicate()
{
    /* nacteni zpravy od uzivatele */
    memset(buf, '\0', BUFSIZE);

    if (connect(client_socket, (const struct sockaddr *) &server_address, sizeof(server_address)) != 0)
    {
        perror("ERROR: connect");
        exit(EXIT_FAILURE);
    }

    memset(buf, '\0', 1024);

    createHTTPHeader();

#ifdef DEBUG
    cout << "======buf:======" << endl;
    cout << buf << endl;
    cout << "=================" << endl;
#endif

    /* odeslani zpravy na server */
    if (send(client_socket, buf, strlen(buf), 0) < 0)
    {
        perror("ERROR in sendto");
        exit(1);
    }

    memset(buf, '\0', 1024);

    /* prijeti odpovedi a jeji vypsani */
    if (recv(client_socket, buf, BUFSIZE, 0) < 0)
    {
        perror("ERROR in recvfrom");
        exit(1);
    }

    readResponse();

    close(client_socket);
    return 0;
}

void Connection::createHTTPHeader()
{
    if (command == "del" || command == "rmd")
        http_header += "DELETE ";
    else if (command == "mkd" || command == "put")
        http_header += "PUT ";
    else if (command == "lst" || command == "get")
        http_header += "GET ";

    http_header += "/" + user + "/" + remote_path + "?";
    http_header += "type=";

    if (command == "mkd" || command == "rmd" || command == "lst")
        http_header += "folder";
    else if (command == "get" || command == "put" || command == "del")
        http_header += "file";

    http_header += " HTTP/1.1\n";

    //  Date  - Timestamp klienta v době vytvoření požadavku.
    http_header += "Date: ";
    char date[255];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    http_header += date;

    // Accept - Požadavaný typ obsahu pro odpověď
    http_header += "\r\nAccept: text/plain";

    // Accept-Encoding - Podporový způsob kódování dat (identity, gzip, deflate)
    http_header += "\r\nAccept-Encoding: identity";

    // Content-Type - MIME typ obsahu požadavku (pro PUT)
    http_header += "\r\nContent-Type: text/plain";

    // Content-Length - Délka obsahu požadavku (pro PUT)
    if (command == "PUT")
        http_header += "\r\nContent-Length: TODO\r\n\r\n";
    else
        http_header += "\r\nContent-Length: 0\r\n\r\n";

    // copy content of HTTP Header into buffer used for sending data to server
    http_header.copy(this->buf, http_header.length());
}

void Connection::readResponse()
{
    string serverResponse(buf);
    string content;

    // parse response code (200, 400, 404) from server response
    int responseCode = atoi(serverResponse.substr(serverResponse.find(" ") + 1, 3).c_str());

    if (responseCode == OK)
    {
        if (command == "lst" )
            // print the listed directories we get from server
            cout << serverResponse.substr(serverResponse.find("\r\n\r\n") + 4) << endl;
    } else {
        // print the error response from server
        cerr << serverResponse.substr(serverResponse.find("\r\n\r\n") + 4);
    }
}




