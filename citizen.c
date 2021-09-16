#include <stdio.h>
#include <stdlib.h>
#include "citizen.h"
#include <string.h>
#include "list.h"


void initCitizen(Citizen *citizen, char *id, char *name, char *surname, char *country, char *virus, char *vacc)
{
    citizen->id = malloc(strlen(id)+1);
    citizen->name = malloc(strlen(name)+1);
    citizen->surname = malloc(strlen(surname)+1);
    citizen->country = malloc(strlen(country)+1);
    citizen->virus = malloc(strlen(virus)+1);
    citizen->vaccinated = malloc(strlen(vacc)+1);
    citizen->date = malloc(sizeof(Date));
}

void setCitizenInfo(Citizen *citizen, char *id, char *name, char *surname, char *country, int age, char *virus, char *vacc, char *date)
{
    strcpy(citizen->id, id);
    strcpy(citizen->name, name);
    strcpy(citizen->surname, surname);
    strcpy(citizen->country, country);
    citizen->age = age;
    strcpy(citizen->virus, virus);
    strcpy(citizen->vaccinated, vacc);
    if(date != NULL)
    setDate(citizen, date);
   
}

void printCitizen(Citizen *citizen)
{
    printf("%s %s %s %s %d %s %s\n", citizen->id, citizen->name, citizen->surname, citizen->country, citizen->age, citizen->virus, citizen->vaccinated);
}

void setDate(Citizen *citizen, char *date)
{
    char *dayPt;
    char *monthPt;
    char *yearPt;
    int day;
    int month;
    int year;

    dayPt = strtok(date, "-");
    monthPt = strtok(NULL, "-");
    yearPt = strtok(NULL, "-");

    if (dayPt != NULL)
    {

        day = atoi(dayPt);
        month = atoi(monthPt);
        year = atoi(yearPt);

        citizen->date->day = day;
        citizen->date->month = month;
        citizen->date->year = year;

        //printf("%d %d %d\n", citizen->date->day, citizen->date->month, citizen->date->year);
    }
}

void initHashTable(HashTable **ht)
{

    HashTable *ht1;
    ht1 = malloc(sizeof(HashTable));

    for (int i = 0; i < HASH_SIZE; i++)
    {

        ht1->array[i] = malloc(sizeof(Citizen));
        ht1->array[i] = NULL;
    }

    *ht = ht1;
}

int hashFunction(char *id)
{
    int citizenId = atoi(id);

    return citizenId % HASH_SIZE;
}

int  insertCitizenHashTable(HashTable *ht, Citizen *citizen, List *country, List *virus, List *vacc)
{

    int index = hashFunction(citizen->id);

    int exist = citizenExist(ht, citizen);
    if(exist == 1) return 0;
    if (exist != 1)
    {
        Citizen *position;
        Citizen *newCitizen;

        newCitizen = malloc(sizeof(Citizen));
        initCitizen(newCitizen, citizen->id, citizen->name, citizen->surname, citizen->country, citizen->virus, citizen->vaccinated);
        strcpy(newCitizen->id, citizen->id);
        strcpy(newCitizen->name, citizen->name);
        strcpy(newCitizen->surname, citizen->surname);
        newCitizen->age = citizen->age;
        newCitizen->country = ptToInfo(country, citizen->country);
        newCitizen->virus = ptToInfo(virus, citizen->virus);
        newCitizen->vaccinated = ptToInfo(vacc, citizen->vaccinated);
        newCitizen->date->day = citizen->date->day;
        newCitizen->date->month = citizen->date->month;
        newCitizen->date->year = citizen->date->year;

        position = ht->array[index];

        if (newCitizen != NULL)
        {

            if (ht->array[index] == NULL)
            {

                ht->array[index] = newCitizen;
                newCitizen->next = NULL;
                
            }
            else
            {

                while (position->next != NULL)
                    position = position->next;

                position->next = newCitizen;
                newCitizen->next = NULL;
            }
            
        }
        return 1;
    }
    return 1;

}

void printHashTable(HashTable *ht)
{

    for (int i = 0; i < HASH_SIZE; i++)
    {

        Citizen *tmp = ht->array[i];

        while (tmp != NULL)
        {
            printf("(%s %s %s)", tmp->id, tmp->virus,tmp->country);

            tmp = tmp->next;
        }
        printf("\n");
    }
}

unsigned long djb2(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;
    while (c = *str++)
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

unsigned long sdbm(unsigned char *str)
{
    unsigned long hash = 0;
    int c;

    while (c = *str++)
    {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

unsigned long hash_i(unsigned char *str, unsigned int i)
{
    return djb2(str) + i * sdbm(str) + i * i;
}

int citizenExist(HashTable *ht, Citizen *citizen)
{
    int index = hashFunction(citizen->id);
    int result1;
    int result2;

    Citizen *tmp;// = malloc(sizeof(Citizen));
    tmp = ht->array[index];

    while (tmp != NULL)
    {
        result1 = strcmp(tmp->id, citizen->id);
        result2 = strcmp(tmp->virus, citizen->virus);

        if ((result1 == 0) && (result2 == 0))
        {

            printf("ERROR IN RECORD ");
            printCitizen(citizen);
            return 1;
        }

        tmp = tmp->next;
    }

    return 0;
}

Citizen* ptToCitizen(HashTable *ht, Citizen *citizen)
{
    int index = hashFunction(citizen->id);
    int result1;
    int result2;

    Citizen *tmp;// = malloc(sizeof(Citizen));
    tmp = ht->array[index];

    while (tmp != NULL)
    {
        result1 = strcmp(tmp->id, citizen->id);
        result2 = strcmp(tmp->virus, citizen->virus);

        if ((result1 == 0) && (result2 == 0))
        {
            return tmp;
        }

        tmp = tmp->next;
    }

    return NULL;
}

void printDate(Citizen* citizen){
    printf("%d-%d-%d \n",citizen->date->day,citizen->date->month,citizen->date->year);
}

void deleteHT(HashTable* ht){
    HashTable* tmp = ht;
    for(int i=0;i<HASH_SIZE;i++){
        Citizen* del;
        while(ht->array[i] != NULL){
            del = ht->array[i];
            ht->array[i] = ht->array[i]->next;
            free(del);
        }
        free(ht->array[i]);
    }
}

void deleteCitizen(Citizen* citizen){
    free(citizen->country);
    free(citizen->date);
    free(citizen->id);
    free(citizen->name);
    free(citizen->surname);
    free(citizen->vaccinated);
    free(citizen->virus);
}

void deleteCitizenHT(HashTable* ht,Citizen* citizen){
    int index = hashFunction(citizen->id);

    Citizen* tmp;
    Citizen* head = ht->array[index];

    if(atoi(head->id) == atoi(citizen->id)){
        tmp = head;
        ht->array[index] = ht->array[index]->next;
        free(tmp);
    }
    else{
        Citizen* current = head;
        while(current->next != NULL){
            if(atoi(current->next->id) == atoi(citizen->id)){
                tmp = current->next;
                current->next = current->next->next;
                free(tmp);
                break;
            }else{
                current = current->next;
            }
        }
    }
}

int idExist(HashTable *ht, char* id)
{
    int index = hashFunction(id);
    int result1;
    int result2;

    Citizen *tmp;// = malloc(sizeof(Citizen));
    tmp = ht->array[index];

    while (tmp != NULL)
    {
        result1 = strcmp(tmp->id,id);

        if (result1 == 0)
        {
            printf("%s %s %s %s\n",tmp->id,tmp->name,tmp->surname,tmp->country);
            printf("AGE: %d\n",tmp->age);
            return 1;
        }

        tmp = tmp->next;
    }

    return 0;
}

Request *initRequests()
{

    Request *request = malloc(sizeof(Request));
    request->head = NULL;
    request->tail = NULL;

    return request;
}

void insertRequest(Request* req,char* cTo,char* date,int result,char* virus){
    
        RequestNode *newNode = malloc(sizeof(RequestNode));
        char* day=strtok(date,"-");
        char* month=strtok(NULL,"-");
        char* year=strtok(NULL,"-");

        int Day=atoi(day);
        int Month=atoi(month);
        int Year=atoi(year);

        newNode->cTo=strdup(cTo);
        newNode->date=malloc(sizeof(Date));
        newNode->virus=strdup(virus);
        newNode->date->day=Day;
        newNode->date->month=Month;
        newNode->date->year=Year;
        newNode->next = NULL;
        if(result == 0){
            newNode->rejected=0;
            newNode->rejected++;
        }
        else{
            newNode->accepted=0;
            newNode->accepted++;
        }
        

        if (req->tail == NULL)
        {
            req->head = newNode;
            req->tail = newNode;
            return;
        }

        req->tail->next = newNode;
        req->tail = newNode;


        
    
}

