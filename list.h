#ifndef LIST_H
#define LIST_H
typedef struct listNode{
    char* name;
    long population;
    int ages[4];
    int accepted;
    int rejected;
    int total;
    int numOfFiles;
    struct List* countriesNames;
    struct listNode* next;
}listNode;

typedef struct List{
    listNode* head;
    listNode* tail;
}List;



typedef struct BloomFilter{
    char* array;
    int size;
    char* virus;
}BloomFilter;

typedef struct BloomNode
{
    BloomFilter* bloom;
    struct BloomNode* next; 
}BloomNode;

typedef struct BloomList{
    BloomNode* head;
    BloomNode* tail;
}BloomList;


List* initList();
BloomList* initBloomList();
int insertNode(List* list,char* name);
void printList(List* cl);
int nodeExist(List* list,char* name);
char* ptToInfo(List* list,char* name);
void initBloomFilter(BloomFilter* bloom,int size,List* list,char* virus);
void bloomInsert(BloomList* blist,char* id,int K,char* virus);
void testBit(BloomFilter* bloom, long position);
void searchBloom(BloomList* list,char* id,int K,char* virus);
void bloomInsertList(BloomList* Blist,BloomFilter* bloom,List* list);
void printBloomList(BloomList* list);
BloomFilter* ptToBloomFilter(BloomList* list,char* virus);
void increasePopulation(List* list,char* name);
void increaseAgeArray(List* list,char* name,int age);
void deleteList(List* list);
void deleteBloomFilter(BloomList* list);
void deleteBloom(BloomFilter* bloom);
void increaseAcceptedRequest(List* list,char* name);
void increaseRejectedRequest(List* list,char* name);
#endif