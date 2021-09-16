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
#include "list.h"
#include "citizen.h"
#include "skipList.h"
#include <sys/epoll.h>

void passingCountriesIntoChilds(char *directory, monitor *child, int numMonitor)
{
    struct dirent *dir;
    DIR *inputDir;

    inputDir = opendir(directory);
    if (inputDir == NULL)
    {
        perror("Unable to read directory1");
        exit(1);
    }
    int counter = 0;
    while ((dir = readdir(inputDir)))
    {

        if ((strcmp(dir->d_name, "..") != 0) && (strcmp(dir->d_name, ".") != 0))
        {

            if (counter == numMonitor)
                counter = 0;
            int exist = insertNode(child[counter].countries, dir->d_name);
            child[counter].numCountries++;
            counter++;
        }
    }
    closedir(inputDir);
}

void initMonitor(monitor *child, int numMonitor, int numFiles)
{
    for (int i = 0; i < numMonitor; i++)
    {
        child[i].Args = (char **)malloc(sizeof(char *) * (numFiles + 12));
        child[i].countries = initList();
        child[i].numCountries = 0;
        child[i].port = 0;
    }
}

int countFiles(char *directory,List* countries)
{
    struct dirent *dir;
    DIR *inputDir;
    int files = 0;
    inputDir = opendir(directory);
    if (inputDir == NULL)
    {
        perror("Unable to read directory1");
        return (1);
    }

    while ((dir = readdir(inputDir)))
    {

        if ((strcmp(dir->d_name, "..") != 0) && (strcmp(dir->d_name, ".") != 0))
        {
            files++;
            int exist=insertNode(countries,dir->d_name);
        }
    }
    closedir(inputDir);

    return files;
}

void passingArguments(monitor *child, char *input_dir, int numMonitor, int port, int numThreads, int bufferSize, int cyclicBuffer, int bloomSize)
{
    char portStr[100];
    char numThreadsStr[100];
    char bufferSizeStr[100];
    char cyclicBufferStr[100];
    char bloomSizeStr[100];
    int tempPort = port;

    //convert integers to strings
    sprintf(portStr, "%d", port);
    sprintf(numThreadsStr, "%d", numThreads);
    sprintf(bufferSizeStr, "%d", bufferSize);
    sprintf(cyclicBufferStr, "%d", cyclicBuffer);
    sprintf(bloomSizeStr, "%d", bloomSize);

    for (int i = 0; i < numMonitor; i++)
    {
        listNode *tmp = child[i].countries->head;
        tempPort++;
        child[i].port = tempPort;
        sprintf(portStr, "%d", tempPort);
        child[i].Args[0] = strdup("./monitorServer");
        child[i].Args[1] = strdup("-p");
        child[i].Args[2] = strdup(portStr);
        child[i].Args[3] = strdup("-t");
        child[i].Args[4] = strdup(numThreadsStr);
        child[i].Args[5] = strdup("-b");
        child[i].Args[6] = strdup(bufferSizeStr);
        child[i].Args[7] = strdup("-c");
        child[i].Args[8] = strdup(cyclicBufferStr);
        child[i].Args[9] = strdup("-s");
        child[i].Args[10] = strdup(bloomSizeStr);

        int j = 0;
        //passing thge paths of the subdirectories
        while (tmp != NULL)
        {
            child[i].Args[11 + j] = malloc(sizeof(char) * 256);
            strcpy(child[i].Args[11 + j], "");
            strncat(child[i].Args[11 + j], input_dir, strlen(input_dir));
            strcat(child[i].Args[11 + j], "/");
            strncat(child[i].Args[11 + j], tmp->name, strlen(tmp->name));
            strncat(child[i].Args[11 + j], "\0", 1);
            j++;
            tmp = tmp->next;
        }
        child[i].Args[11 + child[i].numCountries] = NULL; //terminating argument for execv
    }
}

void printMonitorInfo(monitor *child, int numMonitor)
{
    for (int i = 0; i < numMonitor; i++)
    {
        for (int j = 0; j < (child[i].numCountries + 11); j++)
        {

            printf("%s ", child[i].Args[j]);
        }
        printf("\n");
    }
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

void sendQueries(monitor *child, int *sock, int bufferSize, int numMonitor, int *totals, int *accepted, int *rejected, Request *requests, char *directory,List* countries,int numOfFiles)
{
    char input[100];
    char *cmd;
    int numberForAdd = 100;

    while (1)
    {
        printf("Give us an input\n");
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';

        char *msg = strdup(input);

        cmd = strtok(input, " ");

        if (strcmp(cmd, "/travelRequest") == 0)
        {

            char *id = strtok(NULL, " ");
            char *date = strtok(NULL, " ");
            char *cFrom = strtok(NULL, " ");
            char *cTo = strtok(NULL, " ");
            char *virus = strtok(NULL, " ");
            (*totals)++;

            int temp = 0;

            for (int i = 0; i < numMonitor; i++)
            {
                writeFifo(msg, sock[i], bufferSize);
            }

            int tmp = 0;
            for (int i = 0; i < numMonitor; i++)
            {
                int exist = nodeExist(child[i].countries, cFrom);
                if (exist == 1)
                {
                    tmp = i;
                    break;
                }
            }
            char recMsg[bufferSize];
            

            if (read(sock[tmp], &recMsg, bufferSize) < 0)
            {
                printf("error reading");
            }

            if (strcmp(recMsg, "NO") == 0)
            {
                printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");
                // increaseRejectedRequest(countries, cTo);
                insertRequest(requests, cTo, date, 0, virus);
                (*rejected)++;
            }
            if (strcmp(recMsg, "YES") == 0)
            {
                printf("REQUEST ACCEPTED – HAPPY TRAVELS\n");
                // increaseAcceptedRequest(countries, cTo);
                insertRequest(requests, cTo, date, 1, virus);
                (*accepted)++;
            }

            if (strcmp(recMsg, "NOM") == 0)
            {
                printf("REQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE\n");
                // increaseRejectedRequest(countries, cTo);
                insertRequest(requests, cTo, date, 0, virus);
                (*rejected)++;
            }
        }

        if (strcmp(cmd, "/exit") == 0)
        {
            for (int i = 0; i < numMonitor; i++)
            {
                writeFifo(msg, sock[i], bufferSize);
            }
            char logFile[20];
            strcpy(logFile, "log_file.");
            char pid[20];
            sprintf(pid, "%d", getpid());

            strcat(logFile, pid);

            FILE *fl;
            if ((fl = fopen(logFile, "w")) == NULL)
            {
                printf("Error opening log file \n");
                break;
            }

            listNode* tmp=countries->head;
            while(tmp!=NULL){
                fputs(tmp->name,fl);
                fputs("\n", fl);
                tmp=tmp->next;
            }

            char total[10];
            char accept[10];
            char reject[10];

            sprintf(accept, "%d", (*accepted));
            sprintf(total, "%d", (*totals));
            sprintf(reject, "%d", (*rejected));

            fputs("TOTAL TRAVEL REQUESTS:", fl);
            fputs(total, fl);
            fputs("\n", fl);
            fputs("ACCEPTED: ", fl);
            fputs(accept, fl);
            fputs("\n", fl);
            fputs("REJECTED: ", fl);
            fputs(reject, fl);
            fputs("\n", fl);
            fclose(fl);
            printf("exiting\n");
            break;
        }
        if (!strcmp(cmd, "/travelStats"))
        {
            char *virus = strtok(NULL, " ");
            char *date1 = strtok(NULL, " ");
            char *date2 = strtok(NULL, " ");
            char *country = strtok(NULL, " ");
            int t = 0, a = 0, r = 0;
            char *day1 = strtok(date1, "-");
            char *month1 = strtok(NULL, "-");
            char *year1 = strtok(NULL, "-");
            char *day2 = strtok(date2, "-");
            char *month2 = strtok(NULL, "-");
            char *year2 = strtok(NULL, "-");

            if (country == NULL)
            {
                RequestNode *tmp;
                tmp = requests->head;
                while (tmp != NULL)
                {
                    if (strcmp(tmp->virus, virus) == 0)
                    {
                        int equal = compareDates(day1, month1, year1, day2, month2, year2, tmp->date);
                        if (equal == 1)
                        {

                            t++;
                            if (tmp->accepted == 1)
                                a++;
                            if (tmp->rejected == 1)
                                r++;
                        }
                    }
                    tmp = tmp->next;
                }
                printf("TOTAL REQUEST: %d\n", t);
                printf("ACCEPTED: %d\n", a);
                printf("REJECTED: %d\n", r);
            }
            else
            {
                RequestNode *tmp;
                tmp = requests->head;
                while (tmp != NULL)
                {
                    if ((strcmp(tmp->virus, virus) == 0) && (strcmp(tmp->cTo, country) == 0))
                    {
                        int equal = compareDates(day1, month1, year1, day2, month2, year2, tmp->date);

                        if (equal == 1)
                        {
                            t++;
                            if (tmp->accepted == 1)
                                a++;
                            if (tmp->rejected == 1)
                                r++;
                        }
                    }
                    tmp = tmp->next;
                }
                printf("TOTAL REQUEST: %d\n", t);
                printf("ACCEPTED: %d\n", a);
                printf("REJECTED: %d\n", r);
            }
        }
        if (strcmp(cmd, "/searchVaccinationStatus") == 0)
        {
            char *id = strtok(NULL, " ");
            for (int i = 0; i < numMonitor; i++)
            {
                writeFifo(msg, sock[i], bufferSize);
            }
        }
        if (!strcmp(cmd, "/addVaccinationRecords"))
        {

            char *country = strtok(NULL, " ");
            char path[100];
            strcpy(path, directory);
            strcat(path, "/");
            strcat(path, country);
            strcat(path, "/");
            strcat(path, country);
            char number[10];
            sprintf(number, "%d", numberForAdd);
            strcat(path, "-");
            strcat(path, number);
            strcat(path, ".txt");
            //printf("path is %s\n", path);
            numberForAdd++;

            FILE *file = fopen(path, "w");

            for (int i = 0; i < 10; i++)
            {
                int random = rand() % 1001;
                char id[10];
                sprintf(id, "%d", random);
                fputs(id, file);
                fputs(" KOSTAS ", file);
                fputs("LIAKO ", file);
                fputs(country, file);
                fputs(" 23 ", file);
                fputs("COVID-19 ", file);
                int n = rand() % 2;
                if (n == 0)
                    fputs("NO", file);
                else
                {
                    fputs("YES ", file);
                    fputs("11-6-2021", file);
                }

                fputs("\n", file);
            }

            fclose(file);

            int temp = 0;

            for (int i = 0; i < numMonitor; i++)
            {
                writeFifo(msg, sock[i], bufferSize);
            }
        }
        sleep(1);
    }
}

// int readSocket(int sock, char **reply)
// {
//     char buf[1];
//     strcpy(buf, "");

//     int k = 0;
//     int bytes = 0;
//     if ((k = read(sock, buf, 1)) < 0)
//     {
//         perror("read");
//         exit(1);
//     }
//     bytes = k;

//     (*reply) = malloc((k + 1) * sizeof(char));
//     strcpy((*reply), "");

//     while (k != 0)
//     {
//         if (buf[0] == '\0')
//             break;
//         strncat((*reply), buf, k);
//         if ((k = read(sock, buf, 1)) < 0)
//         {
//             perror("read");
//             exit(1);
//         }
//         if (k != 0)
//         {
//             bytes += k;
//             (*reply) = realloc((*reply), (bytes + k + 1) * sizeof(char));
//         }
//     }
//     strncat((*reply), "\0", 1);
//     return bytes;
// }

// void writeSocket(int sock, char *message)
// {
//     size_t size;

//     size = (strlen(message) + 1) / sizeof(char);
//     for (int i = 0; i < size; i++)
//     {
//         if (write(sock, &message[i], 1) < 0)
//         {
//             perror("write");
//             exit(1);
//         }
//     }
// }