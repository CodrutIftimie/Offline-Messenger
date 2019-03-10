#include "Server.h"

int main(int argc, char* argv[])
{
    Server serv;
    if(serv.Connect((char*)"127.0.0.1", 6768) == true)
    {
        printf("Connection Successful!\n");
        fflush(stdout);
    }
    else
    {
        printf("Connection Failed!\n");
        fflush(stdout);
        exit(1);
    }

    char response;
    write(serv.sd, "KlSv", 5);
    if(read(serv.sd, &response, 1)==0)
    {
        printf("Server closed successfully!\n");
        fflush(stdout);
    }
    else
    {
        printf("Else\n");
        fflush(stdout);
    }
    return 0;
}