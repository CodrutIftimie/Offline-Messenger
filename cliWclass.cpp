#include "cliWclass.h"

bool Server::Connect(char* IP, int PORT)
{
    this->sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1)
    {
        printf("socket() error!\n");
        return false;
    }
    this->con.sin_family = AF_INET;
    this->con.sin_addr.s_addr = inet_addr(IP);
    this->con.sin_port = htons(PORT);
    if (connect(this->sd, (struct sockaddr *)&this->con, sizeof(struct sockaddr)) == -1)
    {
        printf("connect() error!\n");
        return false;
    }
    return true;
}

bool Server::Disconnect()
{
    return close(this->sd);
}