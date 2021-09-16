#ifndef MONITORFUNCTIONS_H
#define MONITORFUNCTIONS_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include "list.h"
#include "citizen.h"
#include "skipList.h"
#define K 16

pthread_mutex_t mutex;
pthread_cond_t nonEmpty;
pthread_cond_t nonFull;
pthread_mutex_t consumeMutex;
pthread_cond_t threadConv;


typedef struct monitor{
    pid_t pid;
    char PR_CW[10];
    char PW_CR[10];
    List* countries;
    int numCountries;
}monitor;

typedef struct shareDataStructure{
    HashTable* ht;
    List* countries;
    List* viruses;
    List* vaccinated;
    listOfSkipList* vaccinatedList;
    listOfSkipList* nonVaccinatedList;
    BloomList* bloomList;
    char** buffer;
    int start;
    int end;
    int count;
    char* input_dir;
}shareDataStructure;

void writeFifo(char* msg,int fd,int buffer);
char* readFifo(char** msg,int fd,int bufferSize);
int moreThan6monthsVaccination(Date* date,char* day);
int numOftxtFiles(char* directory,char* country);
int  insertNodeAndCountFiles(List* countryPath,char* name,int* totalFiles);
shareDataStructure* initDataStructure(int cyclic,char* countryFile);
void placeCountry(shareDataStructure* sh,char* data,int cyclic);
char* consume(shareDataStructure* sh,int cyclic);
int updateData(shareDataStructure* sh,char* countryFile,char* directory,int bloomSize);
int totalViruses(List* viruses);
void parseQuery(shareDataStructure* sh,int bufferSize,int sock,List* txtFiles,char* directory,int bloomSize);
void childServer(int newsock,shareDataStructure* sh,int bufferSize,List* txtFiles,char* directory,int bloomSize);
#endif