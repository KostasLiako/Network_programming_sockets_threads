#ifndef TRAVELFUNCTIONS_H
#define TRAVELFUNCTIONS_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include "list.h"
#include "citizen.h"



typedef struct monitor{
    pid_t pid;
    char PR_CW[10];
    char PW_CR[10];
    List* countries;
    char** Args;
    int numCountries;
    int port;
}monitor;

void writeFifo(char* msg,int fd,int buffer);
char* readFifo(char** msg,int fd,int bufferSize);
int moreThan6monthsVaccination(Date* date,char* day);
int numOftxtFiles(char* directory,char* country);
void passingCountriesIntoChilds(char* dir,monitor* child,int numMonitor);
void initMonitor(monitor* child,int numMonitor,int numFiles);
int countFiles(char* directory,List* countries);
void passingArguments(monitor* child,char* input_dir,int numMonitor,int port,int numThreads,int bufferSize,int cyclicBuffer,int bloomSize);
void printMonitorInfo(monitor* child,int numMonitor);
void sendQueries(monitor* child,int* sock,int bufferSize,int numMonitor,int* totals,int* accepted,int* rejected,Request* requests,char* directory,List* countries,int countFiles);
int readSocket(int sock,char** reply);
void writeSocket(int sock,char* message);
#endif