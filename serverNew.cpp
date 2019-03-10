#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <mysql.h>

#define PORT 6768 
#define MAX_DESC 1020

#define HOST "den1.mysql4.gear.host"
#define USERNAME "messdb"
#define PASSWORD "Tk7gMaAWEW-_"
#define DATABASE "messdb"

extern int errno; 

typedef struct thData { int idThread; int cl; bool stop=false;} thData;
typedef struct descL { int freeDesc[MAX_DESC]; int nofDesc = 0; } descL;
typedef struct onUsers { char uName[50]; int clID;} onUsers;

struct pair
{
    int first;
    int second;
};

descL dL;
onUsers userList[MAX_DESC];
int nofOnUsr = 0;
int sd;

void concat(int argc, ...);

static void *thread(void *);
bool raspunde(void *);
int userPair(int id);
int addDesc(int descID, descL &dL);
int getAvbDesc(descL &dL);
int assignID(char* text);
void addUser(char* username, int clientID);
void removeUser(int clientID);
char *getUsername(int id);
bool isOnline(char* username);
int getClientID(char* username);
void sendFriends(char* username, int idClient);
void sendNewFriend(char* username,char* fromUser, int idClient);
void broadCastOnlineChange(char *username, bool value);
void insertNewMessage(char* message, char* sender, char* receiver);
void sendMessages(int client, char* username);
MYSQL *conn;

int main(int argc, char *argv[])
{
    struct sockaddr_in server; 
    struct sockaddr_in from;

    pthread_t th[100];
    int totalTh=0;

    printf("Initializing MySQL DataBase");
    fflush(stdout);

    conn=mysql_init(NULL);

    if (!conn)
        printf(": FAILED!\n");
    else
        printf(" \xE2\x9C\x93\n");

    printf("Connecting to the DataBase");
    fflush(stdout);
    conn=mysql_real_connect(conn, HOST, USERNAME, PASSWORD , DATABASE ,0,NULL,0);
    if (conn)
        printf(" \xE2\x9C\x93\n");
    else
    {
        printf(": FAILED!\n");
        exit(1);
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("<<<<<<<<<< socket() error >>>>>>>>>>\n");
        return errno;
    }

    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)); 

    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);                         
    server.sin_port = htons(PORT);                                       

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("<<<<<<<<<< bind() error >>>>>>>>>>\n");
        return errno;
    }

    if (listen(sd, 2) == -1)
    {
        perror("<<<<<<<<<< listen() error >>>>>>>>>>\n");
        return errno;
    }
    
    printf("> SERVER started! PORT [%d]\n",PORT);
    while (1)
    {
        int client;
        thData *td;
        int length = sizeof(from);
        fflush(stdout);

        if ((client = accept(sd, (struct sockaddr *)&from, (socklen_t *)&length)) < 0)
        {
            perror("<<<<<<<<<< accept() error >>>>>>>>>>\n");
            continue;
        }

        td = (struct thData *)malloc(sizeof(struct thData));
        if(dL.nofDesc>0)
            td->idThread = getAvbDesc(dL);
        else td->idThread = totalTh++;
        td->cl = client;

        pthread_create(&th[td->idThread], NULL, &thread, td);
    } //while
};

static void *thread(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    printf("[%d] has CONNECTED!\n", tdL.idThread);
    char message[4096], cmd[4], username[50], fromUsername[20];
    bool inLoop = true;

    while (inLoop == true)
    {
        fflush(stdout);
        bzero(&message, sizeof(message));
        int bRead = read(tdL.cl, &message, sizeof(message));
        if (bRead == -1)
        {
            perror("<<<<<<<<<< read() error >>>>>>>>>>\n");
            continue;
        }
        if (bRead == 0)
        {
            printf("[%d] has DISCONNECTED!\n", tdL.idThread);
            fflush(stdout);
            inLoop = false;
            addDesc(tdL.idThread,dL);
            broadCastOnlineChange(getUsername(tdL.cl),false);
            removeUser(tdL.cl);
            break;
        }
        else
        {
            printf("Received [%s] from THREAD [%d]\n", message, tdL.idThread);
            char mes[4000];
            strcpy(cmd,strtok(message," "));
            switch(assignID(cmd))
            {
                case 1:
                {
                    char password[30];
                    strcpy(username,strtok(NULL, " "));
                    strcpy(password,strtok(NULL, ""));
                    bzero(&message,sizeof(message));
                    if(isOnline(username))
                        write(tdL.cl, "2",2);
                    else
                    {
                        sprintf(message,"SELECT uid FROM users WHERE username LIKE BINARY '%s' and password LIKE BINARY '%s'", username, password);
                        if (!mysql_query(conn, message))
                        {
                            MYSQL_RES *result = mysql_store_result(conn);
                            int num_fields = mysql_num_fields(result);
                            if(num_fields!=1 || mysql_fetch_row(result)==NULL)
                            {
                                printf("[%d] has FAILED to LOGIN\n", tdL.idThread);
                                fflush(stdout);
                                write(tdL.cl, "0", 2);
                            }
                            else
                            {
                                printf("[%d] has SUCCEDED to LOGIN\n", tdL.idThread);
                                fflush(stdout);
                                write(tdL.cl, "1", 2);
                                addUser((char*)username,tdL.cl);
                                sendFriends((char*)username, tdL.cl);
                                sleep(1);
                                broadCastOnlineChange((char*)username,true);
                                sendMessages(tdL.cl, (char*)username);
                            }
                        }
                    }
                    break;
                }
                case 2:
                {
                    char password[30], fullName[50];
                    strcpy(username, strtok(NULL," "));
                    strcpy(password, strtok(NULL," "));
                    strcpy(fullName, strtok(NULL, ""));
                    printf("ERR1\n");
                    if(isOnline(username))
                        write(tdL.cl, "0",2);
                    sprintf(message, "SELECT uid FROM users WHERE username LIKE BINARY '%s'", username);
                    mysql_query(conn,message);
                    printf("ERR2\n");
                    MYSQL_RES *result = mysql_store_result(conn);
                    if(mysql_num_rows(result)>0)
                        {write(tdL.cl, "0", 2);printf("ERR3\n");}
                    else
                    {
                        mysql_free_result(result);
                        sprintf(message, "INSERT INTO users(username, password, fullname) VALUES ('%s','%s','%s')", username, password, fullName);
                        mysql_query(conn, message);
                        write(tdL.cl, "1", 2);
                        printf("ERR4\n");
                    }
                    printf("ERR5\n");
                    break;
                }
                case 3:
                {
                    strcpy(username,strtok(NULL," "));
                    strtok(NULL," ");
                    strcpy(fromUsername, strtok(NULL," "));
                    sprintf(mes, "MsFr %s %s", fromUsername, strtok(NULL,""));

                    insertNewMessage(mes,fromUsername, username);
                    printf("ADDED MSG TO DB\n");

                    if(isOnline(username))
                    {
                        if ((bRead = write(getClientID(username),mes, strlen(mes))) <1)
                             perror("<<<<<<<<<< write() error >>>>>>>>>>\n");
                         printf("Wrote [%s] from THREAD [%d] to CLIENT [%d]\n", mes, tdL.idThread, getClientID(username));
                    }
                    else
                    {
                        printf("User [%s] is not online!\n",username);
                    }
                    fflush(stdout);
                    break;
                }
                case 4:
                {
                    char thisU[20];
                    strcpy(username,strtok(NULL," "));
                    strtok(NULL," ");
                    strcpy(thisU,strtok(NULL,""));
                    char q[250];
                    sprintf(q,"SELECT uid FROM users WHERE username LIKE BINARY '%s'", username);
                    if(!mysql_query(conn, q))
                    {
                        MYSQL_RES *result = mysql_store_result(conn);
                        if(mysql_num_rows(result)==1)
                        {
                            mysql_free_result(result);
                            sprintf(q, "SELECT 'existing' FROM users u, users v, friends f WHERE u.uid=f.user and v.uid=f.friendwith and u.username LIKE BINARY '%s' and v.username LIKE BINARY '%s'", username, thisU);
                            mysql_query(conn, q);
                            result = mysql_store_result(conn);
                            if(mysql_num_rows(result)!=1)
                                sendNewFriend(username,thisU,tdL.cl);
                            else write(tdL.cl, "AFFl 1", 7);
                        }
                        else
                            write(tdL.cl, "AFFl 0", 7);
                        mysql_free_result(result);
                    }
                    break;
                }
                case 5:
                {
                    char thisU[20];
                    strcpy(username, strtok(NULL," "));
                    strtok(NULL, " ");
                    strcpy(thisU, strtok(NULL,""));
                    char q[250];
                    sprintf(q, "DELETE FROM friends where user=(SELECT uid from users WHERE username LIKE BINARY '%s') and friendwith=(SELECT uid from users WHERE username LIKE BINARY '%s')", username, thisU);
                    if(mysql_query(conn, q))
                        printf("ERR 1\n");
                    sprintf(q, "DELETE FROM friends where user=(SELECT uid from users WHERE username LIKE BINARY '%s') and friendwith=(SELECT uid from users WHERE username LIKE BINARY '%s')", thisU, username);
                    if(mysql_query(conn, q))
                        printf("ERR 2\n");
                    break;
                }
                case 6:
                {
                    printf("\nShutdown signal!\n");
                    fflush(stdout);
                    mysql_close(conn);
                    close(sd);
                    exit(0);
                    break;
                }
            }
        }
    }
    pthread_detach(pthread_self());
    close((intptr_t)arg);
    return (NULL);
};
int addDesc(int descID, descL &dL)
{
    if (dL.nofDesc + 1 < MAX_DESC)
    {
        int i;
        for (i = 0; dL.freeDesc[i] < descID && i < dL.nofDesc; i++);
        for (int j = dL.nofDesc; j > i; j--)
            dL.freeDesc[j] = dL.freeDesc[j - 1];
        dL.freeDesc[i] = descID;
        dL.nofDesc++;
        return 1;
    }
    return -1;
}

int getAvbDesc(descL &dL)
{
    if (dL.nofDesc > 0)
    {
        int index = 0;
        dL.freeDesc[dL.nofDesc] = dL.freeDesc[0];
        while (index++ < dL.nofDesc)
            dL.freeDesc[index - 1] = dL.freeDesc[index];
        dL.nofDesc--;
        return dL.freeDesc[dL.nofDesc + 1];
    }
    return -1;
}

int assignID(char* text)
{
    if(strcmp(text, "Logi") == 0)
        return 1;
    if(strcmp(text, "Regi") == 0)
        return 2;
    if(strcmp(text, "WrTo") == 0)
        return 3;
    if(strcmp(text, "AdFr") == 0)
        return 4;
    if(strcmp(text, "RmFr") == 0)
        return 5;
    if(strcmp(text, "KlSv") == 0)
        return 6;
    return 0;
}

void addUser(char* username, int clientID)
{
    strcpy(userList[nofOnUsr].uName, username);
    userList[nofOnUsr++].clID = clientID;
}

void removeUser(int clientID)
{
    int i;
    for(i = 0;i < nofOnUsr && userList[i].clID != clientID; i++);
    for(; i < nofOnUsr-1; i++)
    {
        strcpy(userList[i].uName,userList[i+1].uName);
        userList[i].clID=userList[i+1].clID;
    }
    nofOnUsr--;
}

char *getUsername(int id)
{
    for(int i=0; i<nofOnUsr; i++)
        if(userList[i].clID == id)
            return (char*)userList[i].uName;
    return (char*)"NULL";
}

bool isOnline(char* username)
{
    for(int i=0;i<nofOnUsr;i++)
        if(strcmp(userList[i].uName,username)==0)
            return 1;
    return 0;
}

int getClientID(char* username)
{
    for(int i=0;i<nofOnUsr;i++)
        if(strcmp(userList[i].uName,username)==0)
            return userList[i].clID;
    return 0;
}

void sendFriends(char* username, int idClient)
{
    char query[200];
    sprintf(query, "SELECT g.username, g.fullname FROM users u, users g, friends f WHERE u.uid=f.user and g.uid=f.friendwith and u.username LIKE BINARY '%s'", username);
    if(!mysql_query(conn, query))
    {
        MYSQL_RES *result = mysql_store_result(conn);
        MYSQL_ROW row;

        char rows[sizeof(int)];
        sprintf(rows, "%llu", mysql_num_rows(result));

        write(idClient, rows, sizeof(int));
        while ((row = mysql_fetch_row(result)))
        {
            sprintf(query, "%s %s %s", row[0], isOnline(row[0])?"1":"0", row[1]);
            write(idClient, query, sizeof(query));
        }
        mysql_free_result(result);
    }
}

void sendNewFriend(char* username, char* fromUser, int idClient)
{
    char query[200];
    sprintf(query, "SELECT username, fullname FROM users WHERE username LIKE BINARY '%s'", username);
    if(!mysql_query(conn, query))
    {
        MYSQL_RES *result = mysql_store_result(conn);
        MYSQL_ROW row;

        row = mysql_fetch_row(result);
        sprintf(query, "AFSc %s %s %s", row[0], isOnline(row[0])?"1":"0", row[1]);
        write(idClient, query, sizeof(query));
        if(isOnline(username))
        {
            mysql_free_result(result);
            sprintf(query, "SELECT fullname from users where username LIKE BINARY '%s'",fromUser);
            mysql_query(conn, query);
            result = mysql_store_result(conn);
            row = mysql_fetch_row(result);
            sprintf(query, "NwFr %s %s", fromUser, row[0]);
            mysql_free_result(result);
            write(getClientID(username),query, sizeof(query));
        }
        else mysql_free_result(result);

        sprintf(query, "SELECT u.uid,v.uid FROM users u, users v WHERE u.username LIKE BINARY '%s' and v.username LIKE BINARY '%s'", username, fromUser);
        if(!mysql_query(conn,query))
        {
            result = mysql_store_result(conn);
            row = mysql_fetch_row(result);
            char newQuery[100];
            sprintf(newQuery, "INSERT INTO friends VALUES(%s,%s)",row[0],row[1]);
            mysql_query(conn,newQuery);
            sprintf(newQuery, "INSERT INTO friends VALUES(%s,%s)",row[1],row[0]);
            mysql_query(conn,newQuery);
        }
    }
}

void broadCastOnlineChange(char *username, bool value)
{
    if(strlen(username)>0 && strcmp(username,"NULL")!=0)
    {
        char mes[100];
        if(value == false)
            strcpy(mes,"OffL ");
        else strcpy(mes, "OnLn ");
        strcat(mes, username);
        for(int i=0;i<nofOnUsr; i++)
            write(userList[i].clID, mes, sizeof(mes));
        printf("Broadcast [%s] successful!\n",mes);
    }
}

void insertNewMessage(char* message, char* sender, char* receiver)
{
    char query[4100];
    char messageCopy[4000];
    strcpy(messageCopy, message);
    strtok(messageCopy, " ");
    strtok(NULL, " ");
    strcpy(messageCopy,strtok(NULL,""));
    sprintf(query, "INSERT INTO messages(message) VALUES ('%s')",messageCopy);
    if(!mysql_query(conn,query))
    {
        strcpy(query, "SELECT message, mid FROM MESSAGES order by 2 desc");
        mysql_query(conn,query);
        MYSQL_RES *result = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(result);
        int mid = atoi(row[1]);
        mysql_free_result(result);
        sprintf(query, "SELECT u.uid, v.uid FROM users u, users v WHERE u.username LIKE BINARY '%s' and v.username LIKE BINARY '%s'", sender,receiver);
        if(mysql_query(conn, query))
            printf("ERR1\n");
        result = mysql_store_result(conn);
        row = mysql_fetch_row(result);
        sprintf(query, "INSERT INTO redirects(sender, receiver, mid) VALUES (%s,%s,%d)", row[0],row[1],mid);
        if(mysql_query(conn,query))
            printf("ERR2\n");
    }
}

void sendMessages(int client, char* username)
{
    char query[4000];
    sprintf(query, "select distinct message, v.username, u.username from messages m, users u, users v, redirects r where r.mid=m.mid and r.receiver=v.uid and r.sender=u.uid and (v.username like binary '%s' or u.username like binary '%s') order by dateReceived", username, username);
    if(!mysql_query(conn, query))
    {
        MYSQL_RES *result = mysql_store_result(conn);
        MYSQL_ROW row;
        while((row = mysql_fetch_row(result)))
        {
            char message[4000];
            if(strcmp(row[1],username)==0)
                sprintf(message, "MsFr %s %s", row[2], row[0]);
            else sprintf(message, "MsTo %s %s", row[1], row[0]);
            write(client, message, sizeof(message));
        }
    }
}