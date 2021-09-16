#ifndef SKIPLIST_H
#define SKIPLIST_H
#define MAX_LEVEL 4

#include "list.h"
#include "citizen.h"

typedef struct skipNode{
    Citizen* citizen;
    struct skipNode** next;
}skipNode;

typedef struct skipList{
    int level;
    int size;
    char* virus;
    skipNode* head;
}skipList;

typedef struct skipListNode{
    skipList* sList;
    struct skipListNode* next;
}skipListNode;

typedef struct listOfSkipList{
    skipListNode* head;
    skipListNode* tail;
}listOfSkipList;



skipList* skipListInit(List* list,char* virus);
int randLevel();
void insertSkipList(listOfSkipList* list, Citizen *citizen,HashTable* ht);
void printfSkipList(skipList* list);
listOfSkipList* listOfSkipListInit();
void listOfSkipListInsert(listOfSkipList* list,skipList* skipList,List* virusList);
void printListOfSkipList(listOfSkipList* list);
skipList* ptToSkipList(listOfSkipList* list,char* virus);
int  searchSkipList(listOfSkipList* list,char* id,char* virus,char* day);
void searchAllSkipList(listOfSkipList* list,char* id,List* virusList);
void vaccineStatusCitizen(listOfSkipList* list,char* id,char* virus);
void populationStatus(listOfSkipList* list,char* virus,List* counties,char* country,char* d1,char* m1,char* y1,char* d2,char* m2,char* y2);
int compareDates(char* day1,char* month1,char* year1,char* day2,char* month2,char* year2,Date* citizenDate);
void popStatusByAge(listOfSkipList* list,char* virus,List* countries,char* country,char* d1,char* m1,char* y1,char* d2,char* m2,char* y2);
int citizenVaccinated(listOfSkipList* list,char* id,char* virus);
int citizenNoVaccinated(listOfSkipList* list,char* id,char* virus);
void deleteSkipList(listOfSkipList* list,Citizen* citizen);
void nonVaccinated(listOfSkipList* list,char* virus);
#endif