#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include "travelFunctions.h"
#include "citizen.h"
#include "list.h"
#include <time.h>
#include <sys/epoll.h>

int main(int argc, char **argv)
{
    srand(time(NULL));
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;

    if (argc != 13)
    {
        printf("You have an wrong input\n");
        exit(1);
    }

    int numMonitor = atoi(argv[2]);
    int bufferSize = atoi(argv[4]);
    int cyclicBuffer = atoi(argv[6]);
    int bloomSize = atoi(argv[8]);
    char *input_dir = strdup(argv[10]);
    int numThreads = atoi(argv[12]);
    int port = (rand() % 10000) + 10000;
    List* countries=initList();

    monitor child[numMonitor];
    Request* requests=initRequests();

    int numFiles = countFiles(input_dir,countries);
    initMonitor(child, numMonitor, numFiles);
    passingCountriesIntoChilds(input_dir, child, numMonitor);
    passingArguments(child, input_dir, numMonitor, port, numThreads, bufferSize, cyclicBuffer, bloomSize);
    

    pid_t pid;
    for (int i = 0; i < numMonitor; i++) // loop will run n times (n=5)
    {
        if ((pid = fork()) == 0) // child process
        {

            if (execv(child[i].Args[0], child[i].Args) == -1)
            {
                perror("exec");
            }
            exit(0);
        }

        child[i].pid = pid;
    }

    int sock[numMonitor];
    int newsock[numMonitor];
    struct sockaddr_in server, client;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct sockaddr *clientptr = (struct sockaddr *)&client;
    socklen_t clientlen;

    if ((hostname = gethostname(hostbuffer, sizeof(hostbuffer))) == -1)
    {
        perror("gethostname");
        exit(1);
    }

    if ((host_entry = gethostbyname(hostbuffer)) == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }

    IPbuffer = inet_ntoa(*((struct in_addr *)
                               host_entry->h_addr_list[0]));

     sleep(3);

    for (int i = 0; i < numMonitor; i++)
    {

        if ((sock[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("socket");
            exit(1);
        }
        server.sin_family = AF_INET;
        server.sin_port = htons(child[i].port);
        server.sin_addr.s_addr - inet_addr(IPbuffer);



        if (connect(sock[i], serverptr, sizeof(server)) < 0)
        {
            perror("connect");
            exit(1);
        }
    }
   
    int totals=0;
    int accepted=0;
    int rejected=0;

    sendQueries(child,sock,bufferSize,numMonitor,&totals,&accepted,&rejected,requests,input_dir,countries,numFiles);

    for(int i=0;i<numMonitor;i++){
        close(sock[i]);
    }


sleep(3);



    
   

    return 0;
}