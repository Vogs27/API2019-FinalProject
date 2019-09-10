#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAMELENGTH 35
#define RELTYPENUMBER 50
#define ENTITYTABLELENGTH 500
#define RELBLOCKLENGTH 500
#define PROVISIONING 20 //entità e relazioni allocate per volta con malloc

typedef struct relation{  //Una relazione
    char entName[NAMELENGTH];
    struct relation *chained;
}relation;
typedef struct entity{ //Un'entità
    char name[NAMELENGTH];
    unsigned int occ[RELTYPENUMBER];
    relation *outgoingTable[RELTYPENUMBER];
    relation *incomingTable[RELTYPENUMBER];
    struct entity *chained;
}entity;
typedef struct maxEntity{ //un puntatore ad un'entità che è massima
    entity *ptrTo;
    struct maxEntity *chained;
}maxEntity;
typedef struct relType{ //Un tipo di relazione
    char name[NAMELENGTH];
    unsigned int max;
    int needCorrection;
    maxEntity *pointerToList;
}relType;
typedef struct relTOrder{
    unsigned int arrayPos; //posizione della relazione in relTypeTable;
    struct relTOrder *chained;
}relTOrder;

entity entityTable[ENTITYTABLELENGTH];  //Hashtable delle entità
relType relTypeTable[RELTYPENUMBER]; //Hashtable dei tipi di relazione
entity *headOfEntRecycler = NULL; //Cestino delle entità
relation *headOfRelRecycler = NULL; //Cestino delle relazioni
relTOrder *orderQueue = NULL;

void putRel(relation *ptr); //cestina una relazione
void putEnt(entity *ptr); //cestina un'entità
entity *getEnt(); //recupera un'entità dal cestino
relation *getRel(); //recupera una relazione dal cestino
void addEnt(char *name);
void delEnt(char *name);
void addRel(char *from, char *to, char *relName);
void delRel(char *from, char *to, char *relName);
void report();
unsigned int hashEntity(char *name);
unsigned int hashRelType(char *name);
unsigned int hashRelPos(char *from, char *to, char *relName);
unsigned int relTypePos(char *name);
entity *findEnt(char *name);

int main() {
    char cmd[7];
    scanf("%s", cmd);
    while(strcmp(cmd, "end")!=0){
        scanf("%s",cmd);
    }
    return 0;
}

entity *getEnt(){
    if(headOfEntRecycler==NULL){
        headOfEntRecycler=malloc(sizeof(entity)*PROVISIONING);
        for(int i = 0; i<PROVISIONING-1; i++){
            headOfEntRecycler[i].chained = &headOfEntRecycler[i+1];
        }
        headOfEntRecycler[PROVISIONING-1].chained = NULL;
    }
    entity *toReturn = headOfEntRecycler;
    headOfEntRecycler = headOfEntRecycler->chained;
    return toReturn;
}

void putEnt(entity *ptr){
    ptr->chained=headOfEntRecycler;
    headOfEntRecycler=ptr;
}

relation *getRel(){
    if(headOfRelRecycler==NULL){
        headOfRelRecycler = malloc(sizeof(relation)*PROVISIONING);
        for(int i = 0; i<PROVISIONING-1; i++){
            headOfRelRecycler[i].chained = &headOfRelRecycler[i+1];
        }
        headOfRelRecycler[PROVISIONING-1].chained=NULL;
    }
    relation *toReturn = headOfRelRecycler;
    headOfRelRecycler = headOfRelRecycler->chained;
    return toReturn;
}

void putRel(relation *ptr){
    ptr->chained=headOfRelRecycler;
    headOfRelRecycler=ptr;
}

unsigned int hashEntity(char *name){
    unsigned int hash = 0;
    int i;
    for (i = 0 ; name[i] != '\0' ; i++)
    {
        hash = 31*hash + name[i];
    }
    return hash % ENTITYTABLELENGTH;

}

unsigned int hashRelType(char *name){
    unsigned int hash = 0;
    int i;
    for (i = 0 ; name[i] != '\0' ; i++)
    {
        hash = 31*hash + name[i];
    }
    return hash % RELTYPENUMBER;
}

unsigned int hashRelPos(char *from, char *to, char *relName){
    char seed[106];
    strcpy(seed, from);
    strcat(seed, to);
    strcat(seed, relName);
    unsigned int hash = 0;
    int i;
    for (i = 0 ; seed[i] != '\0' ; i++)
    {
        hash = 31*hash + seed[i];
    }
    return hash % RELBLOCKLENGTH;
}

unsigned int relTypePos(char *name){
    unsigned int hashValue = hashRelType(name);
    if(strcmp(relTypeTable[hashValue].name, name)==0){
        return hashValue;
    }else if(relTypeTable[hashValue].name[0] =='\0'){
        strcpy(relTypeTable[hashValue].name, name);
        relTypeTable[hashValue].max=0;
        relTypeTable[hashValue].needCorrection=0;
        relTypeTable[hashValue].pointerToList=NULL;
        //ordino
        if(orderQueue==NULL){
            orderQueue=malloc(sizeof(relTOrder));
            orderQueue->arrayPos = hashValue;
        }
        else if(strcmp(name, relTypeTable[orderQueue->arrayPos].name)<0){
            relTOrder *support = orderQueue;
            orderQueue = malloc(sizeof(relTOrder));
            orderQueue->arrayPos=hashValue;
            orderQueue->chained=support;
        }
        else{
            relTOrder *iterator = orderQueue->chained;
            relTOrder *prev = orderQueue;
            for(; iterator!=NULL; iterator= iterator->chained){
                if(strcmp(name, relTypeTable[iterator->arrayPos].name)<0){
                    prev->chained = malloc(sizeof(relTOrder));
                    prev=prev->chained;
                    prev->chained=iterator;
                    return hashValue;
                }
                prev=iterator;
            }
            prev->chained = malloc(sizeof(relTOrder));
            prev=prev->chained;
            prev->chained=iterator;
        }
        return hashValue;
    } else { //allocazione lineare
        hashValue++;
        while (hashValue < RELTYPENUMBER) {  //NOTA: è un loop infinito, se la tabella è piena, cicla all'infinito alla ricerca di uno spazio!
            if (strcmp(relTypeTable[hashValue].name, name)==0) {
                return hashValue;
            }
            if (relTypeTable[hashValue].name[0] == '\0') {
                break;
            }
            hashValue++;
            if (hashValue == RELTYPENUMBER) {
                hashValue = 0;
            }
        }
        strcpy(relTypeTable[hashValue].name, name);
        relTypeTable[hashValue].max=0;
        relTypeTable[hashValue].needCorrection=0;
        relTypeTable[hashValue].pointerToList=NULL;
        //ordino
        if(orderQueue==NULL){
            orderQueue=malloc(sizeof(relTOrder));
            orderQueue->arrayPos = hashValue;
        }
        else if(strcmp(name, relTypeTable[orderQueue->arrayPos].name)<0){
            relTOrder *support = orderQueue;
            orderQueue = malloc(sizeof(relTOrder));
            orderQueue->arrayPos=hashValue;
            orderQueue->chained=support;
        }
        else{
            relTOrder *iterator = orderQueue->chained;
            relTOrder *prev = orderQueue;
            for(; iterator!=NULL; iterator= iterator->chained){
                if(strcmp(name, relTypeTable[iterator->arrayPos].name)<0){
                    prev->chained = malloc(sizeof(relTOrder));
                    prev=prev->chained;
                    prev->chained=iterator;
                    return hashValue;
                }
                prev=iterator;
            }
            prev->chained = malloc(sizeof(relTOrder));
            prev=prev->chained;
            prev->chained=iterator;
        }
        return hashValue;
    }
}

entity *findEnt(char *name){
    unsigned int hashValue = hashEntity(name);
    if(entityTable[hashValue].name[0]=='\0'){
        return NULL;
    }else if(strcmp(entityTable[hashValue].name, name)==0){
        return &entityTable[hashValue];
    }else{
        entity *iterator = entityTable[hashValue].chained;
        for(; iterator!=NULL; iterator=iterator->chained){
            if(strcmp(iterator->name, name)==0){
                return iterator;
            }
        }
        return NULL;
    }
}

void addRel(char *from, char *to, char *relName){
    entity *fromPtr = findEnt(from);
    if(fromPtr==NULL){
        return;
    }
    entity *toPtr = findEnt(to);
    if(toPtr==NULL){
        return;
    }
    unsigned int typePos = relTypePos(relName);
    unsigned int relPos = hashRelPos(from, to, relName);
    //inserisco la relazione uscente
    if(fromPtr->outgoingTable[typePos]==NULL){
        fromPtr->outgoingTable[typePos]=malloc(sizeof(relation)*RELBLOCKLENGTH);
        relation* relationTable = fromPtr->outgoingTable[typePos];
        for(int i=0; i<RELBLOCKLENGTH; i++){
            relationTable[i].entName[0]= '\0';
            relationTable[i].chained=NULL;
        }
        strcpy(relationTable[relPos].entName,to);
    }
    else if(fromPtr->outgoingTable[typePos][relPos].entName[0]=='\0'){
        strcpy(fromPtr->outgoingTable[typePos][relPos].entName, to);
    }
    else{
        relation *iterator = fromPtr->outgoingTable[typePos][relPos].chained;
        relation *prev = &fromPtr->outgoingTable[typePos][relPos];
        for(; iterator!=NULL; iterator=iterator->chained){
            if(strcmp(iterator->entName, to)==0){
                return;
            }
            prev=iterator;
        }
        prev->chained=getRel();
        prev=prev->chained;
        strcpy(prev->entName, to);
    }
    //inserisco la relazione entrante
    if(toPtr->incomingTable[typePos]==NULL){
        toPtr->incomingTable[typePos]=malloc(sizeof(relation)*RELBLOCKLENGTH);
        relation *relTable = toPtr->incomingTable[typePos];
        for(int i=0; i<RELBLOCKLENGTH; i++){
            relTable[i].entName[0]='\0';
            relTable[i].chained=NULL;
        }
        strcpy(relTable[relPos].entName, from);
    }else if(toPtr->incomingTable[typePos][relPos].entName[0]=='\0'){
        strcpy(toPtr->incomingTable[typePos][relPos].entName, from);
    }else{
        relation *next = toPtr->incomingTable[typePos][relPos].chained;
        toPtr->incomingTable[typePos][relPos].chained = getRel();
        relation *ptr = toPtr->incomingTable[typePos][relPos].chained;
        ptr->chained=next;
        strcpy(ptr->entName, from);
    }
    //Gestione max
}

void delRel(char *from, char *to, char *relName){
    entity *fromPtr = findEnt(from);
    if(fromPtr==NULL){
        return;
    }
    entity *toPtr = findEnt(to);
    if(toPtr==NULL){
        return;
    }
    unsigned int typePos = relTypePos(relName);
    if(fromPtr->outgoingTable[typePos]==NULL){
        return;
    }
    unsigned int relPos = hashRelPos(from, to, relName);

}

void addEnt(char *name){
    unsigned int hashValue = hashEntity(name);
    if(entityTable[hashValue].name[0]=='\0'){
        strcpy(entityTable[hashValue].name, name);
        entityTable[hashValue].chained=NULL;
        for(int i=0; i<RELTYPENUMBER; i++){ //TODO: ottimizzabile ciclando tra le reltype realmente esistenti
            entityTable[hashValue].occ[i]=0;
            entityTable[hashValue].incomingTable[i]=NULL;
            entityTable[hashValue].outgoingTable[i]=NULL;
        }
        return;
    }else if(strcmp(entityTable[hashValue].name, name)==0){
        return;
    }else{
        entity *iterator = entityTable[hashValue].chained;
        entity *prev = &entityTable[hashValue];
        for(; iterator!=NULL; iterator=iterator->chained){
            if(strcmp(iterator->name, name)==0){
                return;
            }
            prev = iterator;
        }
        prev->chained=getEnt();
        strcpy(prev->chained->name, name);
        prev->chained->chained=NULL;
        prev=prev->chained;
        for(int i=0; i<RELTYPENUMBER; i++){ //TODO: ottimizzabile ciclando tra le reltype realmente esistenti
            prev->occ[i]=0;
            prev->incomingTable[i]=NULL;
            prev->outgoingTable[i]=NULL;
        }
        return;
    }
}

void delEnt(char *name){

}

void report(){

}