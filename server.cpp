/*

Vytvoření adresáře bar na serveru bežícím na lokálním počítači a portu 12345:
$ ftrest mkd http://localhost:12345/tonda/foo/bar

Nahrání souboru doc.pdf na serveru do adresáře bar:
$ ftrest put http://localhost:12345/tonda/foo/bar/doc.pdf ~/doc.pdf

Stažení souboru doc.pdf do lokálního adresáře:
$ ftrest get http://localhost:12345/tonda/foo/bar/doc.pdf

Odstranění souboru doc.pdf:
$ ftrest del http://localhost:12345/tonda/foo/bar/doc.pdf

Odstranění adresáře bar:
$ ftrest rmd http://localhost:12345/tonda/foo/bar

 */

#define DEBUG

#include "Server.h"
#include <stdlib.h>

using namespace std;

int main (int argc, char * argv[])
{
    Server * server = new Server();

    int c ;
    while((c = getopt (argc, argv, "r:p:")) != -1 )
    {
        switch(c)
        {
            // root-folder
            case 'r':
                if(optarg)
                    server->root_folder = optarg;
                else
                {
                    cerr << "Missing argument after -r" << endl;
                    delete server;
                }
                break;

            // port
            case 'p':
                if(optarg)
                {
                    server->port_number = atoi(optarg);
                }
                else
                {
                    cerr << "Missing argument after -p" << endl;
                    delete server;
                }
                break;

            default:
                delete server;
                exit(1);
        }
    }

#ifdef DEBUG
    cout << "======DEBUG======" << endl;
    cout << "port: " << server->port_number << endl;
    cout << "root_folder: " << server->root_folder << endl;
    cout << "=================" << endl;
#endif

    server->establishConnection();
    server->doResponse();
    delete server;
}