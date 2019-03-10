#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

class Server
{
public:
    int sd;
    struct sockaddr_in con;
    bool Connect(char* IP, int PORT);
    bool Disconnect();
};