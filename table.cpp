#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <mysql.h>

#define HOST "den1.mysql4.gear.host"
#define USERNAME "messdb"
#define PASSWORD "Tk7gMaAWEW-_"
#define DATABASE "messdb"

int main(int argc, char *argv[])
{
    MYSQL *connect;

    printf("Initializing MySQL DataBase");
    fflush(stdout);

    connect = mysql_init(NULL);

    if (!connect)
        printf(": FAILED!\n");
    else
        printf(" \xE2\x9C\x93\n");

    printf("Connecting to the DataBase");
    fflush(stdout);
    connect = mysql_real_connect(connect, HOST, USERNAME, PASSWORD, DATABASE, 0, NULL, 0);
    if (connect)
        printf(" \xE2\x9C\x93\n");
    else
        printf(": FAILED!\n");

    char message[4000];
    while (read(0, &message, sizeof(message)))
    {
        if (mysql_query(connect, message))
        {
            fprintf(stderr, "%s\n", mysql_error(connect));
        }
        else
        {
             MYSQL_RES *result = mysql_store_result(connect);

            if (result == NULL)
            {
                fprintf(stderr, "%s\n", mysql_error(connect));
            }

            int num_fields = mysql_num_fields(result);
            printf("%d\n", num_fields);

            MYSQL_ROW row;

            while ((row = mysql_fetch_row(result)))
            {
                for (int i = 0; i < num_fields; i++)
                {
                    printf("%s ", row[i] ? row[i] : "NULL");
                }
                printf("\n");
            }

            mysql_free_result(result);
        }
        fflush(stdout);
        bzero(message, sizeof(message));
    }
}