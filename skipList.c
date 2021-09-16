#include <stdio.h>
#include <stdlib.h>
#include "citizen.h"
#include <string.h>
#include "list.h"
#include "skipList.h"
#include "travelFunctions.h"


skipList* skipListInit(List* list,char *virus)
{
    skipList* slist = malloc(sizeof(skipList));
    slist->head = malloc(sizeof(skipNode));
    slist->head->next = (skipNode**)malloc(sizeof(skipNode*)*(MAX_LEVEL+1));

    for(int i=0 ; i < MAX_LEVEL;i++){
        slist->head->next[i] = NULL;
    }
    
    slist->virus = ptToInfo(list,virus);
    slist->level = 0;
    slist->size = 0;
    
    slist->head->citizen = NULL;
    //char* value = "-1";
    // slist->head->citizen->id = malloc(strlen(value)+1);
    // strcpy(slist->head->citizen->id,value);
    return slist;
}

int randLevel()
{
    int level = 0;
    while (rand() < RAND_MAX / 2 && level < MAX_LEVEL)
        level++;
    return level;
}

void insertSkipList(listOfSkipList* list, Citizen *citizen,HashTable* ht)
{
    skipNode *array[MAX_LEVEL + 1];
    
    skipList* slist;
    slist = ptToSkipList(list,citizen->virus);
    if(slist == NULL) return;
    
    skipNode *tmp;
    tmp = slist->head;
    

    for (int i = slist->level; i >= 0; i--)
    {
        while(tmp->next[i] != NULL){
            if(atoi(tmp->next[i]->citizen->id) >= atoi(citizen->id)) break;
            tmp = tmp->next[i];
            
        }
        array[i] = tmp;    
    }

   tmp = tmp->next[0];
    

    if((tmp == NULL) || (atoi(tmp->citizen->id) != atoi(citizen->id))){

        int level = randLevel();

        if(level > slist->level){

            for(int i=slist->level+1;i<level+1;i++){
                array[i] = slist->head;
            }

            slist->level = level;
        }

            skipNode* newNode = malloc(sizeof(skipNode));
            
            newNode->citizen = ptToCitizen(ht,citizen);
            newNode->next = malloc(sizeof(skipNode*)*(level+1));
            // memset(newNode->next,0,sizeof(skipNode*)*(level+1));
            for(int i = 0;i<level;i++){
                newNode->next[i] = NULL;
            }
            
        for(int i=0;i<=level;i++){
            newNode->next[i] = array[i]->next[i];
            array[i]->next[i] = newNode;
        }
        
        slist->size++;
    }
}

void printfSkipList(skipList* list){

    printf("***SKiP LIST***\n");

    for(int i=0;i<MAX_LEVEL;i++){
        skipNode* node = list->head->next[i];
        printf("Level %d :",i);

        while(node != NULL){

            printf("(%d %d)",atoi(node->citizen->id),node->citizen->age);
            node = node->next[i];
        }
        printf("\n");
    }
}

listOfSkipList* listOfSkipListInit(){
    listOfSkipList* list = malloc(sizeof(listOfSkipList));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void listOfSkipListInsert(listOfSkipList* list,skipList* skipList,List* virusList){
    skipListNode* newNode = malloc(sizeof(skipListNode));
    newNode->sList = skipListInit(virusList,skipList->virus);
    newNode->next = NULL;
    
    if(list->tail == NULL){
        list->head = newNode;
        list->tail = newNode;
        return;
    }

    list->tail->next = newNode;
    list->tail = newNode;
}   

void printListOfSkipList(listOfSkipList* list){
    skipListNode* tmp;
    //tmp = malloc(sizeof(skipListNode));
    tmp = list->head;
    while(tmp != NULL){
        printf("%s ",tmp->sList->virus);
        printfSkipList(tmp->sList);
        tmp = tmp->next;
    }
    printf("\n");
}

skipList* ptToSkipList(listOfSkipList* list,char* virus){
    skipListNode* tmp;// = malloc(sizeof(skipListNode));
    tmp = list->head;

    while(tmp != NULL){
        if(strcmp(tmp->sList->virus,virus) == 0)
            return tmp->sList;
        tmp = tmp->next;    
    }

    //printf("Skip List for this virus NOT found\n");
    return NULL;
}
int searchSkipList(listOfSkipList* list,char* id,char* virus,char* day){
    skipList* slist = ptToSkipList(list,virus);
    if(slist == NULL) return 0 ;
    skipNode *tmp = slist->head;

    for (int i = slist->level; i >= 0; i--)
    {
        while((tmp->next[i] != NULL) && (atoi(tmp->next[i]->citizen->id) < atoi(id))){
            tmp = tmp->next[i];
        }
         
    }

    tmp = tmp->next[0];

    if((tmp != NULL) && (strcmp(tmp->citizen->id,id) == 0)){
        
        //printDate(tmp->citizen);
        //printf("\n");
        int noMoreVAcc=moreThan6monthsVaccination(tmp->citizen->date,day);
        if(noMoreVAcc==1) return 2;
        return 1;
       
    }else{
        
        return 0;
        
    }
}

void searchAllSkipList(listOfSkipList* list,char* id,List* virusList){
    
    listNode* tmp;// = malloc(sizeof(listNode));

    tmp = virusList->head;

    while(tmp != NULL){
        vaccineStatusCitizen(list,id,tmp->name);
        tmp = tmp->next;
    }
 }

 void vaccineStatusCitizen(listOfSkipList* list,char* id,char* virus){

    skipList* slist = ptToSkipList(list,virus);
    if(slist == NULL) return;
    skipNode *tmp = slist->head;

    for (int i = slist->level; i >= 0; i--)
    {
        while((tmp->next[i] != NULL) && (atoi(tmp->next[i]->citizen->id) < atoi(id))){
            tmp = tmp->next[i];
        }
         
    }

    tmp = tmp->next[0];

    if((tmp != NULL) && (strcmp(tmp->citizen->id,id) == 0)){
        printf("%s ",virus);
        if(strcmp(tmp->citizen->vaccinated,"YES") == 0){
            printf("VACCINATED ON ");
            printDate(tmp->citizen);
        }else{
            printf("NOT YET VACCINATED");
        }
        printf("\n");
    }
}

void populationStatus(listOfSkipList* list,char* virus,List* country,char* countryName,char* day1,char* month1,char* year1,char* day2,char* month2,char* year2){
    skipList* slist = ptToSkipList(list,virus);
    if(slist == NULL) return;
    skipNode *tmp = slist->head;
    int counter=0;
    double percent=0.0;
    listNode *current;// = malloc(sizeof(listNode));
    current = country->head; 


    while ((current != NULL))
    {
        if(strcmp(current->name,countryName) == 0) break;
        current = current->next;
    }


    tmp = tmp->next[0];

    while(tmp != NULL ){
        
        if(strcmp(tmp->citizen->country,countryName) == 0){ 
            if(compareDates(day1,month1,year1,day2,month2,year2,tmp->citizen->date) ==1)
            counter++;
            
        }
        tmp = tmp->next[0];
    }

    
    printf("\n");
    percent = (double)counter/(double)current->population;
    percent = percent * 100;
    printf("%s %d %f %%\n",countryName,counter,percent);
}

int compareDates(char* day1,char* month1,char* year1,char* day2,char* month2,char* year2,Date* citizenDate){


    if((citizenDate->year > atoi(year1)) && (citizenDate->year < atoi(year2))) return 1;
    if(citizenDate->year == atoi(year1)){
        if(citizenDate->month > atoi(month1)) return 1;
        if(citizenDate->month == atoi(month1)){
            if(citizenDate->day >= atoi(day1)) return 1;
        }
    }
    if(citizenDate->year == atoi(year2)){
        if(citizenDate->month < atoi(month2)) return 1;
        if(citizenDate->month == atoi(month1)){
            if(citizenDate->day <= atoi(day1)) return 1;
        }
    }
    return 0;
}

void popStatusByAge(listOfSkipList* list,char* virus,List* country,char* countryName,char* d1,char* m1,char* y1,char* d2,char* m2,char* y2){
    skipList* slist = ptToSkipList(list,virus);
    if(slist == NULL) return;
    skipNode *tmp = slist->head;
    int counter[4];
    double percent[4];
    listNode *current;// = malloc(sizeof(listNode));
    current = country->head; 

    for(int i =0 ; i < 4; i++){
        counter[i] = 0;
        percent[i] = 0.0;
    }

    while ((current != NULL))
    {
        if(strcmp(current->name,countryName) == 0) break;
        current = current->next;
    }

    tmp = tmp->next[0];

    while(tmp != NULL ){
        
        if(strcmp(tmp->citizen->country,countryName) == 0){ 
            if(compareDates(d1,m1,y1,d2,m2,y2,tmp->citizen->date) ==1){
                if((tmp->citizen->age >= 0) && (tmp->citizen->age<20)) counter[0]++;
                if((tmp->citizen->age >=20) && (tmp->citizen->age<40)) counter[1]++;
                if((tmp->citizen->age >=40) && (tmp->citizen->age<60)) counter[2]++;
                if((tmp->citizen->age >= 60)) counter[3]++;
            }

            
        }
        tmp = tmp->next[0];
    }

    printf("\n");
    for(int i =0;i<4;i++){
        if(current->ages[i] != 0){
            percent[i] = (double)counter[i]/(double)current->ages[i];
        }else{
            percent[i] =0.0;
        }
        percent[i] = percent[i] * 100;
    }
    printf("%s \n",countryName);
    printf("0-20 %d %f %% \n",counter[0],percent[0]);
    printf("20-40 %d %f %%\n",counter[1],percent[1]);
    printf("40-60 %d %f %%\n",counter[2],percent[2]);
    printf("60+ %d %f %%\n",counter[3],percent[3]);
}

int citizenVaccinated(listOfSkipList* list,char* id,char* virus){

    skipList* slist = ptToSkipList(list,virus);
    if(slist == NULL) return 0;
    skipNode *tmp = slist->head;

    for (int i = slist->level; i >= 0; i--)
    {
        while((tmp->next[i] != NULL) && (atoi(tmp->next[i]->citizen->id) < atoi(id))){
            tmp = tmp->next[i];
        }
         
    }

    tmp = tmp->next[0];

    if((tmp != NULL) && (strcmp(tmp->citizen->id,id) == 0)){
        printf("ERROR: CITIZEN %s ALREADY VACCINATED ON ",tmp->citizen->id);
        printDate(tmp->citizen);
        printf("\n");
        return 1;
    }else{
        return 0;
    }
}

int citizenNoVaccinated(listOfSkipList* list,char* id,char* virus){

    skipList* slist = ptToSkipList(list,virus);
    if(slist == NULL) return 0;
    skipNode *tmp = slist->head;

    for (int i = slist->level; i >= 0; i--)
    {
        while((tmp->next[i] != NULL) && (atoi(tmp->next[i]->citizen->id) < atoi(id))){
            tmp = tmp->next[i];
        }
         
    }

    tmp = tmp->next[0];

    if((tmp != NULL) && (strcmp(tmp->citizen->id,id) == 0)){
        return 1;
    }else{
        return 0;
    }
}

void deleteSkipList(listOfSkipList* list,Citizen* citizen){
    skipNode *array[MAX_LEVEL + 1];
    
    skipList* slist;// = malloc(sizeof(skipList));
    slist = ptToSkipList(list,citizen->virus);
    if(slist == NULL) return;
    
    skipNode *tmp;// = malloc(sizeof(skipNode));
    tmp = slist->head;


    for (int i = slist->level; i >= 0; i--)
    {
        while((tmp->next[i] != NULL) && (atoi(tmp->next[i]->citizen->id) < atoi(citizen->id))){
            tmp = tmp->next[i];
        }
        array[i] = tmp;    
    }

   tmp = tmp->next[0];

   if((tmp != NULL) && (atoi(tmp->citizen->id) == atoi(citizen->id))){

       for(int i=0;i<= slist->level;i++){

           if(array[i]->next[i] != tmp) break;

           array[i]->next[i] = tmp->next[i];
       }

       while((slist->level > 0) && (slist->head->next[slist->level] == 0)){
           slist->level--;
       }
   }
}

void nonVaccinated(listOfSkipList* list,char* virus){
    skipList* slist = ptToSkipList(list,virus);
    if(slist == NULL) return;
    skipNode *tmp = slist->head;


    tmp = tmp->next[0];

    while(tmp != NULL ){
        
        printf("%s %s %s %s %d \n",tmp->citizen->id,tmp->citizen->name,tmp->citizen->surname,tmp->citizen->country,tmp->citizen->age);
        tmp = tmp->next[0];
    }
}

// void destroySkipList(listOfSkipList* list){
//     skipListNode* tmp;

//     while(list->head != NULL){
//         tmp = list->head;
//         list->head = list->head->next;

//         skipNode* curr = tmp->sList->head;
//         curr = curr->next[0];
//         skipNode* del;
//         while(curr != NULL){
//             del = curr;
//             curr = curr->next;
//             free(del->citizen);
//         }

//     }
// }