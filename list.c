#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "citizen.h"

List *initList()
{

    List *list = malloc(sizeof(List));
    list->head = NULL;
    list->tail = NULL;

    return list;
}

BloomList* initBloomList(){
    BloomList* list = malloc(sizeof(BloomList));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void bloomInsertList(BloomList* Blist,BloomFilter* bloom,List* list){
    BloomNode* newNode = malloc(sizeof(BloomNode));
    newNode->bloom = malloc(sizeof(BloomFilter));
    initBloomFilter(newNode->bloom,bloom->size,list,bloom->virus);
    newNode->next = NULL;

    if(Blist->tail == NULL){
        Blist->head = newNode;
        Blist->tail = newNode;

    }

    Blist->tail->next = newNode;
    Blist->tail = newNode;

}

void printBloomList(BloomList* list){
    BloomNode * tmp;
    //tmp = malloc(sizeof(BloomNode));
    tmp = list->head;
    while(tmp != NULL){

        printf("%s ",tmp->bloom->virus);
        tmp = tmp->next;
    }
    printf("\n");
}

int insertNode(List *list, char *name)
{
    int exist = nodeExist(list, name);
    if (exist != 1)
    {
        listNode *newNode = malloc(sizeof(listNode));
        int len = strlen(name);
        newNode->name = malloc(strlen(name)+1);
        strcpy(newNode->name, name);
        newNode->next = NULL;
        newNode->population = 0;
        for(int i=0;i<4;i++){
            newNode->ages[i] = 0;
        }

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

void printList(List *cl)
{

    listNode *tmp;
    // tmp = malloc(sizeof(listNode));
    tmp = cl->head;

    while (tmp != NULL)
    {
        printf("(%s)",tmp->name);
        tmp = tmp->next;
    }
    free(tmp);
    printf("\n");
}

int nodeExist(List *list, char *name)
{

    listNode *tmp; //= malloc(sizeof(listNode));
    tmp = list->head;

    while (tmp != NULL)
    {

        if (strcmp(tmp->name, name) == 0)
            return 1;
        tmp = tmp->next;
    }
    return 0;
}

char* ptToInfo(List* list,char* name){
    listNode *tmp;// = malloc(sizeof(listNode));
    tmp = list->head;

    while (tmp != NULL)
    {

        if (strcmp(tmp->name, name) == 0)
            return tmp->name;
        tmp = tmp->next;
    }
    printf("Country not Found\n");
    return NULL;
}

void initBloomFilter(BloomFilter* bloom,int size,List* list,char* virus){
    
    bloom->size = size;
    bloom->array = malloc(sizeof(char)*size);
    memset(bloom->array,0,sizeof(char)*size);
    bloom->virus = ptToInfo(list,virus);
    //printf("virus %s \n",bloom->virus);
}

BloomFilter* ptToBloomFilter(BloomList* list,char* virus){
    
    BloomNode* tmp;// = malloc(sizeof(BloomNode));
    tmp = list->head;

    while (tmp != NULL)
    {

        if (strcmp(tmp->bloom->virus,virus) == 0)
            return tmp->bloom;
        tmp = tmp->next;
    }
    printf("BloomFilter for this Virus not Found\n");
    return NULL;
}

void bloomInsert(BloomList* list,char* id,int K,char* virus){
    BloomFilter* bloom;
    bloom = ptToBloomFilter(list,virus);
    if(bloom == NULL){ printf("ERROR\n"); exit(1);}
    long hash; 
    for(int i=0;i<K;i++){
        
    hash = hash_i(id,i);
    hash = hash % (bloom->size*8);
    bloom->array[hash/8] |= 1 << (hash%8);
    }
}

void testBit(BloomFilter* bloom,long pos){
    if((bloom->array[pos/8] & (1 << (pos%8))) != 0)
        printf("Bit %ld is 1\n",pos);
    else
        printf("Bit %ld is 0\n",pos);    

      
}

void searchBloom(BloomList* list,char* id,int K,char* virus){
    BloomFilter* bloom;
    bloom = ptToBloomFilter(list,virus);
    if(bloom == NULL){ printf("ERROR\n"); exit(1);}
    long hash;
    for(int i=0;i<K;i++){
        hash = hash_i(id,i);
        hash = hash % (bloom->size*8);

        if((bloom->array[hash/8] & (1 << (hash%8))) == 0){
            printf("NOT VACCINATED\n");
            return;
        }

    }
    printf("MAYBE VACCINATED\n");
    return;
}

void increasePopulation(List* list,char* name){
    listNode *tmp;// = malloc(sizeof(listNode));
    tmp = list->head;

    while (tmp != NULL)
    {

        if (strcmp(tmp->name, name) == 0) tmp->population++;
            
        tmp = tmp->next;
    }
    
}

void increaseAgeArray(List* list,char* name,int age){

    listNode *tmp;// = malloc(sizeof(listNode));
    tmp = list->head;

    while (tmp != NULL)
    {

        if (strcmp(tmp->name, name) == 0) break;
            
        tmp = tmp->next;
    }
    
    if((age >= 0) && (age<20)) tmp->ages[0]++;
    if((age >=20) && (age<40)) tmp->ages[1]++;
    if((age >=40) && (age<60)) tmp->ages[2]++;
    if((age >= 60)) tmp->ages[3]++;

}

void deleteList(List* list){
    listNode* tmp;

    while(list->head != NULL){
        tmp = list->head;
        list->head = list->head->next;
        free(tmp);
    }
}

void deleteBloomFilter(BloomList* list){
    BloomNode* tmp;

    while(list->head != NULL){
        tmp = list->head;
        list->head = list->head->next;
        deleteBloom(tmp->bloom);
        free(tmp);
    }
}

void deleteBloom(BloomFilter* bloom){
    free(bloom->array);
}

void increaseAcceptedRequest(List* list,char* name){
    listNode *tmp;// = malloc(sizeof(listNode));
    tmp = list->head;

    while (tmp != NULL)
    {

        if (strcmp(tmp->name, name) == 0) {
            
        tmp->accepted++;
        tmp->total++;
        }
            
        tmp = tmp->next;
    }
    
}


void increaseRejectedRequest(List* list,char* name){
    listNode *tmp;// = malloc(sizeof(listNode));
    tmp = list->head;

    while (tmp != NULL)
    {

        if (strcmp(tmp->name, name) == 0) {
            
        tmp->rejected++;
        tmp->total++;
        }
            
        tmp = tmp->next;
    }
    
}