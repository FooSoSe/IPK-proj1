#include <stdlib.h>

#include "Connection.h"

int main (int argc, const char * argv[])
{
    if (argc != 3)
    {
        cerr << "usage: ftrest <command> /USER-ACCOUNT/REMOTE-PATH?type=[file|folder] HTTP/1.1\n" << endl;
        exit(EXIT_FAILURE);
    }

    Connection * connection = new Connection(argv[1], argv[2]);
    connection->establishConnection();
    connection->communicate();
    delete connection;

    return 0;
}