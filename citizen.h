#ifndef CITIZEN_H
#define CITIZEN_H
#define HASH_SIZE 10

#include "list.h"

typedef struct Date{
    int day;
    int month;
    int year;
}Date;

typedef struct Citizen {
    char* id;
    char* name;
    char* surname;
    char* country;
    int age;
    char* virus;
    char* vaccinated;
    Date* date;

    struct Citizen* next;
}Citizen;

typedef struct HashTable{
    Citizen* array[HASH_SIZE];
}HashTable;

typedef struct RequestNode{
    char* cFrom;
    char* cTo;
    char* virus;
    Date* date;
    int accepted;
    int rejected;
    struct RequestNode* next;
}RequestNode;

typedef struct Request{
    RequestNode* head;
    RequestNode* tail;
}Request;




void initCitizen(Citizen* citizen,char* id,char* name,char* surname,char* country,char* virus,char* vacc);
void setCitizenInfo(Citizen* citizen,char* id,char* name,char* surname,char* country,int age,char* virus,char* vacc,char* date);
void printCitizen(Citizen* citizen);
void setDate(Citizen* citizen,char* date);
void initHashTable(HashTable** ht);
int insertCitizenHashTable(HashTable* ht,Citizen* citizen,List* country,List* virus,List* vacc);
int hashFunction(char* id);
void printHashTable(HashTable* ht);
int citizenExist(HashTable* ht,Citizen* citizen);
Citizen* ptToCitizen(HashTable *ht, Citizen *citizen);
void printDate(Citizen* citizen);
void deleteHT(HashTable* ht);
void deleteCitizen(Citizen* citizen);
void deleteCitizenHT(HashTable* ht,Citizen* citizen);
int idExist(HashTable *ht, char* id);
Request *initRequests();
void insertRequest(Request* req,char* cTo,char* date,int result,char* virus);

unsigned long djb2(unsigned char *str);
unsigned long sdbm(unsigned char *str);
unsigned long hash_i(unsigned char *str, unsigned int i);

#endif