#include "cliWclass.h"

int main(int argc, char *argv[])
{
    Server srv;
    srv.Connect((char*)"127.0.0.1",6768);

    bool hasLoggedIn = false;
    while (1)
    {
        if(hasLoggedIn == false)
        {
            char username[50];
            char password[20];
            char info[100];
            printf("Username:");
            fflush(stdout);
            bzero(&info, sizeof(info));
            bzero(&username, sizeof(username));
            bzero(&password, sizeof(password));
            read(0, &username, sizeof(username));
            username[strlen(username) - 1] = '\0';
            read(0, &password, sizeof(password));
            password[strlen(password) - 1] = '\0';
            strcpy(info, username);
            strcat(info, " ");
            strcat(info, password);
            write(srv.sd, info, sizeof(info));
            printf("Wrote [%s] to server\n", info);
            fflush(stdout);
            bzero(&info, sizeof(info));
            read(srv.sd, &info, sizeof(info));
            if(strcmp(info,"1")==0)
            {
                printf("Login successful!\n");
                fflush(stdout);
                hasLoggedIn = true;
            }
            else
            {
                printf("Login failed!\n");
                fflush(stdout);
            }
        }
        else
        {
            char message[4096];
            printf("Message:");
            fflush(stdout);
            bzero(&message, sizeof(message));
            read(0, &message, sizeof(message));
            message[strlen(message) - 1] = '\0';
            write(srv.sd, message, sizeof(message));
            printf("Wrote [%s] to server\n", message);
            bzero(&message, sizeof(message));
            read(srv.sd, &message, sizeof(message));
            printf("Received from server: %s\n", message);
        }
    }
    srv.Disconnect();
}
