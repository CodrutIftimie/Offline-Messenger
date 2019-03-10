#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

class Server
{
public:
    int sd;
    struct sockaddr_in con;
    bool Connect(char* IP, int PORT);
    void Disconnect();
};

#endif // SERVER_H
