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
#include "list.h"
#include "citizen.h"
#include <errno.h>
#include "monitorFunctions.h"

char **buffer;
extern pthread_mutex_t mutex;
extern pthread_mutex_t consumeMutex;
extern pthread_cond_t nonEmpty;
extern pthread_cond_t nonFull;
extern pthread_cond_t threadConv;
int totalFiles = 0;
int cyclicBufferFiles;
shareDataStructure *shDataStr;
int globalBloomSize = 0;
int readF = 0;
int writeF = 0;
int exitFlag = 0;
int totalFilesToRead = 0;
int numThreads = 0;
char countryRead[100];

//entry point of threads
void *threadFunc(void *arg)
{
    while ((totalFiles > 0) || (shDataStr->count > 0))
    {
        pthread_cond_wait(&threadConv, &consumeMutex);

        if (exitFlag == 1)
            break;
        strcpy(countryRead, consume(shDataStr, cyclicBufferFiles));
        int i = updateData(shDataStr, countryRead, shDataStr->input_dir, globalBloomSize);
        readF++;
        pthread_cond_signal(&nonFull);
        usleep(5000);
    }
    exitFlag = 1;
    for (int i = 0; i < numThreads; i++)
    {
        pthread_cond_signal(&threadConv);
        pthread_mutex_unlock(&consumeMutex);
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{

    //printf("Child with pid %d\n", getpid());

    int port = atoi(argv[2]);
    numThreads = atoi(argv[4]);
    int bufferSize = atoi(argv[6]);
    int cyclicBuffer = atoi(argv[8]);
    cyclicBufferFiles = cyclicBuffer;
    int bloomSize = atoi(argv[10]);
    globalBloomSize = bloomSize;
    List *countries = initList();
    List* txtFiles=initList();
    pthread_t *tids;
    char **allCountries;
    char *temp = strdup(argv[11]);
    char *directory = strtok(temp, "/");
    int numOfViruses = 0;
    int opt = 1;
    int err;

    //initialize mutexes, condiotion variables and data structures how save the data
    shDataStr = initDataStructure(cyclicBuffer, directory);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&consumeMutex, NULL);
    pthread_cond_init(&nonEmpty, NULL);
    pthread_cond_init(&nonFull, NULL);
    pthread_cond_init(&threadConv, NULL);

    struct dirent *dir;
    DIR *inputDir;

    //count how much files exists in subdirectories
    for (int i = 11; i < argc; i++)
    {
        int exist = insertNodeAndCountFiles(countries, argv[i], &totalFiles);
    }
    totalFilesToRead = totalFiles;
    allCountries = (char **)malloc(sizeof(char *) * totalFiles);

    //set all txt files in array
    int counter = 0;
    for (int i = 11; i < argc; i++)
    {

        inputDir = opendir(argv[i]);
        if (inputDir == NULL)
        {
            perror("Unable to read directory1");
            exit(1);
        }
        while ((dir = readdir(inputDir)))
        {

            if ((strcmp(dir->d_name, "..") != 0) && (strcmp(dir->d_name, ".") != 0))
            {
                allCountries[counter] = strdup(dir->d_name);
                int exist=insertNode(txtFiles,dir->d_name);
                counter++;
            }
        }
        closedir(inputDir);
    }

    struct sockaddr_in server, client;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct sockaddr *clientptr = (struct sockaddr *)&client;
    socklen_t clientlen;
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;
    int sock;
    int newsock;


    //sleep(3);

    if ((tids = malloc(numThreads * sizeof(pthread_t))) == NULL)
    {
        perror("Malloc");
        exit(1);
    }
    //creating threads
    for (int i = 0; i < numThreads; i++)
    {
        if (err = pthread_create(tids + i, NULL, threadFunc, (void *)shDataStr))
        {
            printf("Error create thread\n");
            exit(1);
        }
    }

    //parent-producer thread fill the cyclic
    counter = 0;
    while ((totalFiles > 0) || (shDataStr->count > 0))
    {
        if (totalFiles > 0)
        {
            placeCountry(shDataStr, allCountries[counter], cyclicBuffer);
            //printf("i placed %s and count %d\n",allCountries[counter],shDataStr->count);
            totalFiles--;
            writeF++;
            pthread_cond_signal(&nonEmpty);
        }
        //remaining countries in counter
        if ((shDataStr->count > 0) || (totalFiles > 0))
        {
            pthread_cond_signal(&threadConv);
        }
        usleep(1000);
        counter++;
    }



    //sleep(5);
    for (int i = 0; i < numThreads; i++)
    {
        if (err = pthread_join(*(tids + i), NULL))
        {
            printf("Error join thread\n");
            exit(1);
        }
    }
  
    numOfViruses = totalViruses(shDataStr->viruses);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (bind(sock, serverptr, sizeof(server)) < 0)
    {
        perror("bind");
        exit(1);
    }
    if (listen(sock, 5) < 0)
    {
        perror("listen");
        exit(1);
    }

    while(1){
    if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
    {
        perror("accept");
        exit(1);
    }
    printf("Accept connection in port %d\n",port);

    switch(fork()){
        case -1: perror("fork"); break;
        case 0: close(sock); childServer(newsock,shDataStr,bufferSize,txtFiles,directory,bloomSize);
        exit(0);
    }
    close(newsock);
    break;
    }
    
    

   // printf("Exiting process %d\n",getpid());
    close(newsock);

    
    pthread_cond_destroy(&nonFull);
    pthread_cond_destroy(&nonEmpty);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&consumeMutex);
    pthread_cond_destroy(&threadConv);


    return 0;
}