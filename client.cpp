#include <stdlib.h>

#include "Connection.h"

int main (int argc, const char * argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: ftrest <command> /USER-ACCOUNT/REMOTE-PATH?type=[file|folder]\n" << endl;
        exit(EXIT_FAILURE);
    }

    Connection connection(argv[1], argv[2]);
    connection.establishConnection();
    connection.communicate();

    return 0;
}