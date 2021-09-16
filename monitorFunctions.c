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
#include "citizen.h"
#include "list.h"
#include "monitorFunctions.h"

int insertNodeAndCountFiles(List *list, char *name, int *totalfiles)
{
    int exist = nodeExist(list, name);
    if (exist != 1)
    {
        listNode *newNode = malloc(sizeof(listNode));
        int len = strlen(name);
        newNode->name = malloc(strlen(name) + 1);
        strcpy(newNode->name, name);
        newNode->next = NULL;
        newNode->population = 0;
        for (int i = 0; i < 4; i++)
        {
            newNode->ages[i] = 0;
        }
        newNode->numOfFiles = 0;
        struct dirent *dir;
        DIR *inputDir;
        newNode->countriesNames = initList();

        inputDir = opendir(name);
        if (inputDir == NULL)
        {
            perror("Unable to read directory1");
            exit(1);
        }
        while ((dir = readdir(inputDir)))
        {

            if ((strcmp(dir->d_name, "..") != 0) && (strcmp(dir->d_name, ".") != 0))
            {
                newNode->numOfFiles++;
                (*totalfiles)++;
                int exist = insertNode(newNode->countriesNames, dir->d_name);
            }
        }
        closedir(inputDir);

        if (list->tail == NULL)
        {
            list->head = newNode;
            list->tail = newNode;
            return 1;
        }

        list->tail->next = newNode;
        list->tail = newNode;
        return 1;
    }
    return 0;
}

shareDataStructure *initDataStructure(int cyclicBuffer, char *directory)
{
    shareDataStructure *data = malloc(sizeof(shareDataStructure));
    data->countries = initList();
    data->viruses = initList();
    data->vaccinated = initList();
    data->vaccinatedList = listOfSkipListInit();
    data->nonVaccinatedList = listOfSkipListInit();
    data->bloomList = initBloomList();
    initHashTable(&data->ht);
    data->buffer = malloc(sizeof(char *) * cyclicBuffer);
    data->count = 0;
    data->end = -1;
    data->start = 0;
    data->input_dir = strdup(directory);
    return data;
}

void writeFifo(char *input, int fd, int buffer)
{

    size_t size;
    int chunks;
    int lastBytes;
    int i;

    size = (strlen(input) + 1);
    lastBytes = size % buffer; //who much bytes is less than buffer size
    chunks = size / buffer;    //split message into chunks

    for (i = 0; i < (chunks + 1); i++)
    { //send message (chunks +1) times

        if (lastBytes == 0 && i < chunks)
        {
            write(fd, &input[i * buffer], buffer);
        }

        else if (lastBytes != 0)
        {

            if (i == chunks)
                write(fd, &input[i * buffer], lastBytes);
            else
                write(fd, &input[i * buffer], buffer);
        }
    }
}

char *readFifo(char **input, int fd, int bufferSize)
{
    char buffer[bufferSize];
    int i;
    int bytesForRead;
    strcpy(buffer, "");

    i = read(fd, buffer, bufferSize);
    bytesForRead = i;

    (*input) = malloc((i + 1) * sizeof(char));
    strcpy((*input), "");
    while (i != 0)
    {
        strncat((*input), buffer, i);
        i = read(fd, buffer, bufferSize); //when i==0 we havent more bytes to read
        if (i != 0)
        {
            bytesForRead += i;
            (*input) = realloc((*input), (bytesForRead + i + 1) * sizeof(char));
        }
    }
    strncat((*input), "\0", 1); //set the terminate string character
    return (*input);
}

int moreThan6monthsVaccination(Date *date, char *travelday)
{
    char *day = strtok(travelday, "-");
    char *month = strtok(NULL, "-");
    char *year = strtok(NULL, "-");

    int Day = atoi(day);
    int Month = atoi(month);
    int Year = atoi(year);

    if ((Year - date->year) >= 2)
        return 1;
    if (Year == date->year)
    {
        if ((Month - date->month) > 6)
            return 1;
    }
    return 0;
}

int numOftxtFiles(char *directory, char *country)
{
    char *tmpdir = strdup(directory);
    strcat(tmpdir, "/");
    strcat(tmpdir, country);
    int counter = 0;
    struct dirent *dir;
    DIR *inputDir = opendir(tmpdir);
    if (inputDir == NULL)
    {
        perror("Unable to read directory1");
        return (1);
    }
    while ((dir = readdir(inputDir)))
    {

        if ((strcmp(dir->d_name, "..") != 0) && (strcmp(dir->d_name, ".") != 0))
        {
            //printf("dir->name %s\n",dir->d_name);
            counter++;
        }
    }
    closedir(inputDir);
    return counter;
}

void placeCountry(shareDataStructure *sh, char *data, int cyclic)
{
    pthread_mutex_lock(&mutex);
    while (sh->count >= cyclic)
    {
        printf("Found buffer full\n");
        //pthread_cond_signal(&threadConv);
        pthread_cond_wait(&nonFull, &mutex);
    }
    sh->end = (sh->end + 1) % cyclic;
    sh->buffer[sh->end] = strdup(data);
    sh->count++;
    pthread_mutex_unlock(&mutex);
}

char *consume(shareDataStructure *sh, int cyclic)
{
    char *data;
    data = malloc(sizeof(char) * 30);
    strcpy(data, "");
    pthread_mutex_lock(&mutex);
    while (sh->count <= 0)
    {
        printf("Found buffer empty\n");
        //pthread_cond_signal(&nonFull);
        pthread_cond_wait(&nonEmpty, &mutex);
    }
    strcpy(data, sh->buffer[sh->start]);
    sh->start = (sh->start + 1) % cyclic;
    sh->count--;
    pthread_mutex_unlock(&mutex);
    return data;
}

int updateData(shareDataStructure *sh, char *countryFile, char *directory, int bloomSize)
{
    Citizen *citizen;
    char line[256];
    BloomFilter *BloomFilter;
    skipList *slist;

    char *tmp = strdup(countryFile);
    char *country = strtok(tmp, "-");
    char path[100];
    strcpy(path, "");
    strcat(path, directory);
    strcat(path, "/");
    strcat(path, country);
    strcat(path, "/");
    strcat(path, countryFile);
    strcat(path, "\0");
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open file");
    }
    BloomFilter = malloc(sizeof(BloomFilter));
    while (fgets(line, sizeof(line), file) != NULL)
    {
        citizen = malloc(sizeof(Citizen));

        char *id = strtok(line, " ");
        char *name = strtok(NULL, " ");
        char *surname = strtok(NULL, " ");
        char *country = strtok(NULL, " ");
        char *citizenAge = strtok(NULL, " ");
        char *virus = strtok(NULL, " ");
        char *vacc = strtok(NULL, " ");
        char *date = strtok(NULL, " ");
        int age = atoi(citizenAge);

        initCitizen(citizen, id, name, surname, country, virus, vacc);
        setCitizenInfo(citizen, id, name, surname, country, age, virus, vacc, date);
        int countryExist = insertNode(sh->countries, country);
        int virusExist = insertNode(sh->viruses, virus);
        int vaccinated = insertNode(sh->vaccinated, vacc);

        int insert = insertCitizenHashTable(sh->ht, citizen, sh->countries, sh->viruses, sh->vaccinated);
        if (insert != 0)
        {
            increasePopulation(sh->countries, country);
            increaseAgeArray(sh->countries, country, age);
            if (virusExist == 1)
            {
                initBloomFilter(BloomFilter, bloomSize, sh->viruses, virus);
                slist = skipListInit(sh->viruses, virus);
                bloomInsertList(sh->bloomList, BloomFilter, sh->viruses);
                listOfSkipListInsert(sh->nonVaccinatedList, slist, sh->viruses);
                listOfSkipListInsert(sh->vaccinatedList, slist, sh->viruses);
            }
            //----INSERT BLOOM FILTER---
            if (strcmp(citizen->vaccinated, "YES") == 0)
            {
                bloomInsert(sh->bloomList, citizen->id, K, citizen->virus);
                insertSkipList(sh->vaccinatedList, citizen, sh->ht);
            }
            else
            {
                insertSkipList(sh->nonVaccinatedList, citizen, sh->ht);
            }
        }

        free(citizen);
        //deleteCitizen(citizen);
    }

    // printf("path is %s \n", path);
    return 0;
}

int totalViruses(List *viruses)
{
    int count = 0;
    listNode *tmp = viruses->head;
    while (tmp != NULL)
    {
        count++;
        tmp = tmp->next;
    }
    return count;
}

void parseQuery(shareDataStructure *sh, int bufferSize, int sock, List *txtFiles, char *directory,int bloomSize)
{
    char *recMsg = malloc(sizeof(char) * 100);
    strcpy(recMsg, "");
    while (1)
    {
        sleep(1);
        read(sock, recMsg,100);
        char *cmd;

        cmd = strtok(recMsg, " ");

        if (!strcmp(cmd, "/travelRequest"))
        {

            char *id = strtok(NULL, " ");
            char *date = strtok(NULL, " ");
            char *cFrom = strtok(NULL, " ");
            char *cTo = strtok(NULL, " ");
            char *virus = strtok(NULL, " ");
            //printf("recMsg is %s\n", recMsg);

            int exist = nodeExist(sh->countries, cFrom);
            if (exist == 0)
            {
                //printf("DEN THN VRHKA\n");
                // write(sock,"ABORT",strlen("ABORT"));
            }
            else
            {
               // printList(sh->countries);

                int answer = searchSkipList(sh->vaccinatedList, id, virus, date);

                if (answer == 1)
                {

                    if (write(sock, "YES", strlen("YES")) == -1)
                        printf("error writting\n");
                }
                else if (answer == 2)
                {

                    write(sock, "NOM", strlen("NOM"));
                }
                else
                {
                    write(sock, "NO", strlen("NO"));
                }
            }
        }
        if (strcmp(cmd, "/exit") == 0)
        {
            printf("exiting monitor server %d\n",getpid());
            break;
        }
        if (!strcmp(cmd, "/searchVaccinationStatus"))
        {
            char *id = strtok(NULL, " ");
            int exist = idExist(sh->ht, id);
            if (exist == 1)
            {
                searchAllSkipList(sh->vaccinatedList, id, sh->viruses);
                searchAllSkipList(sh->nonVaccinatedList, id, sh->viruses);
            }
        }
        if (!strcmp(cmd, "/addVaccinationRecords"))
        {
            listNode *tmp;
            tmp = sh->countries->head;
            char path[100];
            struct dirent *dir;
            DIR *inputDir;

            char* country=strtok(NULL," ");
            int countryExist=nodeExist(sh->countries,country);
        if(countryExist==1){
            while (tmp != NULL)
            {
                strcpy(path, directory);
                strcat(path, "/");
                strcat(path, tmp->name);
                inputDir=opendir(path);
                if(inputDir == NULL){
                    perror("Unable to read directory");
                    exit(1);
                }
                while(dir=readdir(inputDir)){
                    if ((strcmp(dir->d_name, "..") != 0) && (strcmp(dir->d_name, ".") != 0))
                    {
                        int exist=nodeExist(txtFiles,dir->d_name);
                        if(exist==0){
                            int upadated=updateData(sh,dir->d_name,directory,bloomSize);
                            if(upadated==0){
                                printf("Data upadated\n");
                            }
                        }
                    }
                }
                closedir(inputDir);
                tmp=tmp->next;
            }
        }
        }
    }
}

void childServer(int newsock, shareDataStructure *sh, int bufferSize, List *txtFiles, char *directory,int bloomSize)
{
    //printf("Im child and accept connection\n");

    parseQuery(sh, bufferSize, newsock, txtFiles, directory,bloomSize);

    sleep(2);

    close(newsock);
}