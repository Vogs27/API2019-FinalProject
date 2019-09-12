//
// Created by alessandro on 12/09/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAMELENGTH 35
#define RELTYPENUMBER 50
#define ENTITYTABLELENGTH 500
#define RELBLOCKLENGTH 500

typedef struct relatedEntity{  //Una relazione
    char entName[NAMELENGTH];
    struct relatedEntity *chained;
}relatedEntity;

typedef struct entity{ //Un'entità
    char name[NAMELENGTH];
    unsigned int occ[RELTYPENUMBER];
    relatedEntity **outgoingTable[RELTYPENUMBER];
    relatedEntity **incomingTable[RELTYPENUMBER];
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

entity *entityTable[ENTITYTABLELENGTH];  //Hashtable delle entità
relType relTypeTable[RELTYPENUMBER]; //Hashtable dei tipi di relazione
entity *headOfEntRecycler = NULL; //Cestino delle entità
struct relatedEntity *headOfRelRecycler = NULL; //Cestino delle relazioni
relTOrder *orderQueue = NULL;

void putRel(relatedEntity *ptr); //cestina una relazione
void putEnt(entity *ptr); //cestina un'entità
entity *getEnt(); //recupera un'entità dal cestino
relatedEntity *getRel(); //recupera una relazione dal cestino
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
    freopen("/home/alessandro/Scaricati/batch2.1.in","r",stdin);
    char cmd[7];
    scanf("%s", cmd);
    while(strcmp(cmd, "end")!=0){
        if(strcmp(cmd, "addent")==0){
            char name[NAMELENGTH];
            scanf(" \"%[^\"]\"", name);
            addEnt(name);
        }
        else if(strcmp(cmd, "addrel")==0){
            char from[NAMELENGTH];
            char to[NAMELENGTH];
            char type[NAMELENGTH];
            scanf(" \"%[^\"]\"", from);
            scanf(" \"%[^\"]\"", to);
            scanf(" \"%[^\"]\"", type);
            addRel(from, to, type);
        }
        else if(strcmp(cmd, "report")==0){
            report();
        }
        else if(strcmp(cmd, "delent")==0){
            char name[NAMELENGTH];
            scanf(" \"%[^\"]\"", name);
            delEnt(name);
        }
        else if(strcmp(cmd, "delrel")==0){
            char from[NAMELENGTH];
            char to[NAMELENGTH];
            char type[NAMELENGTH];
            scanf(" \"%[^\"]\"", from);
            scanf(" \"%[^\"]\"", to);
            scanf(" \"%[^\"]\"", type);
            delRel(from, to, type);
        }
        scanf("%s",cmd);
    }
    return 0;
}

entity *getEnt(){
    if(headOfEntRecycler==NULL){
        headOfEntRecycler=malloc(sizeof(entity));
        headOfEntRecycler->chained=NULL;
    }
    entity *toReturn = headOfEntRecycler;
    toReturn->chained=NULL;
    toReturn->name[0]='\0';
    headOfEntRecycler = headOfEntRecycler->chained;
    return toReturn;
}

void putEnt(entity *ptr){
     ptr->chained=headOfEntRecycler;
     headOfEntRecycler=ptr;
}

relatedEntity *getRel(){
    if(headOfRelRecycler==NULL){
        headOfRelRecycler = malloc(sizeof(relatedEntity));
        headOfRelRecycler->chained=NULL;
    }
    relatedEntity *toReturn = headOfRelRecycler;
    headOfRelRecycler = headOfRelRecycler->chained;
    toReturn->chained=NULL;
    toReturn->entName[0]='\0';
    return toReturn;
}

void putRel(relatedEntity *ptr){
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

entity *findEnt(char *name){
    unsigned int hash = hashEntity(name);
    if(entityTable[hash]==NULL){
        return NULL;
    }
    if(strcmp(entityTable[hash]->name, name)==0){
        return entityTable[hash];
    }else if(entityTable[hash]->chained!=NULL){
        for(entity *iterator = entityTable[hash]->chained; iterator!=NULL; iterator=iterator->chained){
            if(strcmp(iterator->name, name)==0){
                return iterator;
            }
        }
    }
    return NULL;
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

unsigned int relTypePos(char *name) { //Done
    unsigned int hashValue = hashRelType(name);
    if (strcmp(relTypeTable[hashValue].name, name) == 0) {
        return hashValue;
    } else if (relTypeTable[hashValue].name[0] == '\0') {
        strcpy(relTypeTable[hashValue].name, name);
        relTypeTable[hashValue].max = 0;
        relTypeTable[hashValue].needCorrection = 0;
        relTypeTable[hashValue].pointerToList = NULL;
        //ordino
        if (orderQueue == NULL) {
            orderQueue = malloc(sizeof(relTOrder));
            orderQueue->arrayPos = hashValue;
            orderQueue->chained = NULL;
        } else if (strcmp(name, relTypeTable[orderQueue->arrayPos].name) < 0) {
            relTOrder *support = orderQueue;
            orderQueue = malloc(sizeof(relTOrder));
            orderQueue->arrayPos = hashValue;
            orderQueue->chained = support;
        } else {
            relTOrder *iterator = orderQueue->chained;
            relTOrder *prev = orderQueue;
            for (; iterator != NULL; iterator = iterator->chained) {
                if (strcmp(name, relTypeTable[iterator->arrayPos].name) < 0) {
                    prev->chained = malloc(sizeof(relTOrder));
                    prev = prev->chained;
                    prev->chained = iterator;
                    prev->arrayPos = hashValue;
                    return hashValue;
                }
                prev = iterator;
            }
            prev->chained = malloc(sizeof(relTOrder));
            prev = prev->chained;
            prev->chained = iterator;
            prev->arrayPos = hashValue;
        }
        return hashValue;
    } else { //allocazione lineare
        hashValue++;
        while (hashValue <
               RELTYPENUMBER) {  //NOTA: è un loop infinito, se la tabella è piena, cicla all'infinito alla ricerca di uno spazio!
            if (strcmp(relTypeTable[hashValue].name, name) == 0) {
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
        relTypeTable[hashValue].max = 0;
        relTypeTable[hashValue].needCorrection = 0;
        relTypeTable[hashValue].pointerToList = NULL;
        //ordino
        if (orderQueue == NULL) {
            orderQueue = malloc(sizeof(relTOrder));
            orderQueue->arrayPos = hashValue;
        } else if (strcmp(name, relTypeTable[orderQueue->arrayPos].name) < 0) {
            relTOrder *support = orderQueue;
            orderQueue = malloc(sizeof(relTOrder));
            orderQueue->arrayPos = hashValue;
            orderQueue->chained = support;
        } else {
            relTOrder *iterator = orderQueue->chained;
            relTOrder *prev = orderQueue;
            for (; iterator != NULL; iterator = iterator->chained) {
                if (strcmp(name, relTypeTable[iterator->arrayPos].name) < 0) {
                    prev->chained = malloc(sizeof(relTOrder));
                    prev = prev->chained;
                    prev->chained = iterator;
                    prev->arrayPos = hashValue;
                    return hashValue;
                }
                prev = iterator;
            }
            prev->chained = malloc(sizeof(relTOrder));
            prev = prev->chained;
            prev->chained = iterator;
            prev->arrayPos = hashValue;
        }
        return hashValue;
    }
}

void addEnt(char *name){
    unsigned int entPos = hashEntity(name);
    if(entityTable[entPos]==NULL){
        entityTable[entPos]= getEnt();
        entityTable[entPos]->chained=NULL;
        strcpy(entityTable[entPos]->name, name);
        for(int i=0; i<RELTYPENUMBER; i++){
            entityTable[entPos]->occ[i]=0;
            entityTable[entPos]->incomingTable[i]=NULL;
            entityTable[entPos]->outgoingTable[i]=NULL;
        }
    }else{
        for(entity *check=entityTable[entPos]; check!=NULL; check=check->chained){
            if(strcmp(check->name, name)==0){
                return;
            }
        }
        entity *toBeMoved = entityTable[entPos];
        entityTable[entPos]= getEnt();
        entityTable[entPos]->chained=toBeMoved;
        strcpy(entityTable[entPos]->name, name);
        for(int i=0; i<RELTYPENUMBER; i++){
            entityTable[entPos]->occ[i]=0;
            entityTable[entPos]->incomingTable[i]=NULL;
            entityTable[entPos]->outgoingTable[i]=NULL;
        }
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
    //aggiungo relazione uscente
    if(fromPtr->outgoingTable[typePos]==NULL){
        fromPtr->outgoingTable[typePos]=malloc(sizeof(relatedEntity*)*RELBLOCKLENGTH);
        relatedEntity** relationTable = fromPtr->outgoingTable[typePos];
        for(int i=0; i<RELBLOCKLENGTH; i++){
            relationTable[i]=NULL;
        }
        relationTable[relPos]=malloc(sizeof(relatedEntity));
        strcpy(relationTable[relPos]->entName,to);
    }
    else{
        relatedEntity *iterator = fromPtr->outgoingTable[typePos][relPos];
        for(; iterator!=NULL; iterator=iterator->chained){
            if(strcmp(iterator->entName, to)==0){
                return;
            }
        }
        relatedEntity *head = fromPtr->outgoingTable[typePos][relPos];
        fromPtr->outgoingTable[typePos][relPos] = getRel();
        fromPtr->outgoingTable[typePos][relPos]->chained=head;
        strcpy(fromPtr->outgoingTable[typePos][relPos]->entName, to);
    }
    //aggiungo relazione entrante
    if(toPtr->incomingTable[typePos]==NULL){
        toPtr->incomingTable[typePos]=malloc(sizeof(relatedEntity*)*RELBLOCKLENGTH);
        relatedEntity** relationTable = toPtr->incomingTable[typePos];
        for(int i=0; i<RELBLOCKLENGTH; i++){
            relationTable[i]=NULL;
        }
        relationTable[relPos]=malloc(sizeof(relatedEntity));
        strcpy(relationTable[relPos]->entName,from);
    }
    else{
        relatedEntity *head = toPtr->incomingTable[typePos][relPos];
        toPtr->incomingTable[typePos][relPos] = getRel();
        toPtr->incomingTable[typePos][relPos]->chained=head;
        strcpy(toPtr->incomingTable[typePos][relPos]->entName, from);
    }
    //gestione massimo
    toPtr->occ[typePos]++;
    if(relTypeTable[typePos].needCorrection==0){
        if(relTypeTable[typePos].max==toPtr->occ[typePos]){
            if(strcmp(toPtr->name, relTypeTable[typePos].pointerToList->ptrTo->name)<0){
                maxEntity *next = relTypeTable[typePos].pointerToList;
                relTypeTable[typePos].pointerToList=malloc(sizeof(maxEntity));
                relTypeTable[typePos].pointerToList->chained = next;
                relTypeTable[typePos].pointerToList->ptrTo=toPtr;
            }else{
                maxEntity *list = relTypeTable[typePos].pointerToList->chained;
                maxEntity *prev = relTypeTable[typePos].pointerToList;
                for(;list!=NULL; list=list->chained){
                    if(strcmp(toPtr->name,list->ptrTo->name)<0){
                        break;
                    }
                    prev=list;
                }
                prev->chained=malloc(sizeof(maxEntity));
                prev=prev->chained;
                prev->chained=list;
                prev->ptrTo=toPtr;
            }
            return;
        }
        if(toPtr->occ[typePos]>relTypeTable[typePos].max){
            while(relTypeTable[typePos].pointerToList!=NULL){ //cancello i massimi già esistenti
                maxEntity *temp=relTypeTable[typePos].pointerToList->chained;
                free(relTypeTable[typePos].pointerToList);
                relTypeTable[typePos].pointerToList = temp;
            }
            relTypeTable[typePos].pointerToList=malloc(sizeof(maxEntity)); //aggiungo il nuovo massimo
            relTypeTable[typePos].pointerToList->chained=NULL;
            relTypeTable[typePos].pointerToList->ptrTo=toPtr;
            relTypeTable[typePos].max=toPtr->occ[typePos];
            return;
        }
    }

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
    unsigned int typePos = hashRelType(relName);
    while(typePos < RELTYPENUMBER) {  //NOTA: è un loop infinito, se la tabella è piena, cicla all'infinito alla ricerca di typePos!
        if (strcmp(relTypeTable[typePos].name, relName) == 0) {
            break;
        }
        if (relTypeTable[typePos].name[0] == '\0') {
            return;
        }
        typePos++;
        if (typePos == RELTYPENUMBER) {
            typePos = 0;
        }
    }

    if(fromPtr->outgoingTable[typePos]==NULL){ //se non esiste la tabella delle relazioni uscenti, non può esistere la relazione
        return;
    }

    if(toPtr->incomingTable[typePos]==NULL){ //se non esiste la tabella delle relazioni entranti, non può esistere la relazione
        return;
    }

    unsigned int relPos = hashRelPos(from, to, relName);

    if(toPtr->incomingTable[typePos][relPos]==NULL){ //se non esiste un puntatore nella tabella delle relazioni entranti, non può esistere la relazione
        return;
    }

    //Cancello la relazione uscente
    relatedEntity *out = fromPtr->outgoingTable[typePos][relPos];
    relatedEntity *prev = NULL;
    for(; out!=NULL; out=out->chained){
        if(strcmp(out->entName, to)==0){
            break;
        }
        prev=out;
    }
    if(out==NULL){//se arrivo in fondo senza trovare nulla
        return;
    }
    if(prev==NULL){ //se sono ancora all'inizio
        prev=out->chained;
        free(fromPtr->outgoingTable[typePos][relPos]);
        fromPtr->outgoingTable[typePos][relPos]=prev;
    }else{
        prev->chained=out->chained;
        free(out);
    }
    //cancello la relazione entrante
    relatedEntity *in = toPtr->incomingTable[typePos][relPos];
    relatedEntity *previn = NULL;
    for(; in!=NULL; in=in->chained){
        if(strcmp(in->entName, from)==0){
            break;
        }
        previn=in;
    }

    if(previn==NULL){ //se sono ancora all'inizio
        previn=in->chained;
        free(toPtr->incomingTable[typePos][relPos]);
        toPtr->incomingTable[typePos][relPos]=previn;
    }else{
        previn->chained=in->chained;
        free(in);
    }
    //aggiorno i massimi
    if(relTypeTable[typePos].needCorrection==0) {
        if (toPtr->occ[typePos] == relTypeTable[typePos].max) {
            if (relTypeTable[typePos].pointerToList->chained == NULL) {
                relTypeTable[typePos].needCorrection = 1;
            } else {
                if (relTypeTable[typePos].pointerToList->ptrTo == toPtr) {
                    maxEntity *temp = relTypeTable[typePos].pointerToList->chained;
                    free(relTypeTable[typePos].pointerToList);
                    relTypeTable[typePos].pointerToList = temp;
                } else {
                    maxEntity *iterator = relTypeTable[typePos].pointerToList->chained;
                    maxEntity *prevMax = relTypeTable[typePos].pointerToList;
                    for (; iterator != NULL; iterator = iterator->chained) {
                        if (iterator->ptrTo == toPtr) {
                            maxEntity *temp = iterator->chained;
                            free(iterator);
                            prevMax->chained = temp;
                            break;
                        }
                        prevMax = iterator;
                    }
                }
            }
        }
    }
    toPtr->occ[typePos]--;
}

void delEnt(char *name){
    unsigned int hash = hashEntity(name);
    entity *entPtr = entityTable[hash];
    entity *prevEnt = NULL;
    for(; entPtr!=NULL; entPtr=entPtr->chained){
        if(strcmp(entPtr->name, name)==0){
            break;
        }
        prevEnt=entPtr;
    }
    if(entPtr==NULL){
        return;
    }
    for(relTOrder *relTord = orderQueue; relTord!=NULL; relTord=relTord->chained){
        //verifico se entPtr è un massimo
        if(relTypeTable[relTord->arrayPos].needCorrection==0) {
            if (entPtr->occ[relTord->arrayPos] == relTypeTable[relTord->arrayPos].max) {
                if (relTypeTable[relTord->arrayPos].pointerToList->chained == NULL) {
                    relTypeTable[relTord->arrayPos].needCorrection = 1;
                } else {
                    if (relTypeTable[relTord->arrayPos].pointerToList->ptrTo == entPtr) {
                        maxEntity *temp = relTypeTable[relTord->arrayPos].pointerToList->chained;
                        free(relTypeTable[relTord->arrayPos].pointerToList);
                        relTypeTable[relTord->arrayPos].pointerToList = temp;
                    } else {
                        maxEntity *iterator = relTypeTable[relTord->arrayPos].pointerToList->chained;
                        maxEntity *prevMax = relTypeTable[relTord->arrayPos].pointerToList;
                        for (; iterator != NULL; iterator = iterator->chained) {
                            if (iterator->ptrTo == entPtr) {
                                maxEntity *temp = iterator->chained;
                                free(iterator);
                                prevMax->chained = temp;
                                break;
                            }
                            prevMax = iterator;
                        }
                    }
                }
            }
        }




        //Cancello le relazioni uscenti da entPtr:
        if(entPtr->outgoingTable[relTord->arrayPos]!=NULL){
            for(int relPos = 0; relPos < RELBLOCKLENGTH; relPos++){
                while(entPtr->outgoingTable[relTord->arrayPos][relPos]!=NULL){
                    relatedEntity *next= entPtr->outgoingTable[relTord->arrayPos][relPos]->chained;
                    entity *toPtr = findEnt(entPtr->outgoingTable[relTord->arrayPos][relPos]->entName);
                    relatedEntity *in = toPtr->incomingTable[relTord->arrayPos][relPos];
                    relatedEntity *previn = NULL;
                    for(; in!=NULL; in=in->chained){
                        if(strcmp(in->entName, name)==0){
                            break;
                        }
                        previn=in;
                    }
                    if(previn==NULL){ //se sono ancora all'inizio
                        previn=in->chained;
                        free(toPtr->incomingTable[relTord->arrayPos][relPos]);
                        toPtr->incomingTable[relTord->arrayPos][relPos]=previn;
                    }else{
                        previn->chained=in->chained;
                        free(in);
                    }
                    //aggiorno i massimi
                    if(relTypeTable[relTord->arrayPos].needCorrection==0) {
                        if (toPtr->occ[relTord->arrayPos] == relTypeTable[relTord->arrayPos].max) {
                            if (relTypeTable[relTord->arrayPos].pointerToList->chained == NULL) {
                                relTypeTable[relTord->arrayPos].needCorrection = 1;
                            } else {
                                if (relTypeTable[relTord->arrayPos].pointerToList->ptrTo == toPtr) {
                                    maxEntity *temp = relTypeTable[relTord->arrayPos].pointerToList->chained;
                                    free(relTypeTable[relTord->arrayPos].pointerToList);
                                    relTypeTable[relTord->arrayPos].pointerToList = temp;
                                } else {
                                    maxEntity *iterator = relTypeTable[relTord->arrayPos].pointerToList->chained;
                                    maxEntity *prevMax = relTypeTable[relTord->arrayPos].pointerToList;
                                    for (; iterator != NULL; iterator = iterator->chained) {
                                        if (iterator->ptrTo == toPtr) {
                                            maxEntity *temp = iterator->chained;
                                            free(iterator);
                                            prevMax->chained = temp;
                                            break;
                                        }
                                        prevMax = iterator;
                                    }
                                }
                            }
                        }
                    }
                    toPtr->occ[relTord->arrayPos]--;
                    free(entPtr->outgoingTable[relTord->arrayPos][relPos]);
                    entPtr->outgoingTable[relTord->arrayPos][relPos]=next;
                }
            }
        }
        //cancello le relazioni entranti
        if(entPtr->incomingTable[relTord->arrayPos]!=NULL){
            for(int relPos = 0; relPos<RELBLOCKLENGTH; relPos++){
                while(entPtr->incomingTable[relTord->arrayPos][relPos]!=NULL){
                    relatedEntity *next = entPtr->incomingTable[relTord->arrayPos][relPos]->chained;
                    entity *fromPtr = findEnt(entPtr->incomingTable[relTord->arrayPos][relPos]->entName);
                    relatedEntity *out = fromPtr->outgoingTable[relTord->arrayPos][relPos];
                    relatedEntity *prevout=NULL;
                    for(;out!=NULL; out=out->chained){
                        if(strcmp(out->entName, name)==0){
                            break;
                        }
                        prevout=out;
                    }
                    if(prevout==NULL){
                        prevout=out->chained;
                        free(fromPtr->outgoingTable[relTord->arrayPos][relPos]);
                        fromPtr->outgoingTable[relTord->arrayPos][relPos] = prevout;
                    }else{
                        prevout->chained=out->chained;
                        free(out);
                    }
                    free(entPtr->incomingTable[relTord->arrayPos][relPos]);
                    entPtr->incomingTable[relTord->arrayPos][relPos]=next;
                }
            }
            entPtr->occ[relTord->arrayPos]=0;
        }
    }
    //cancello entPtr
    if(prevEnt==NULL){ //se è il puntatore iniziale
        prevEnt=entPtr->chained;
        free(entPtr);
        entityTable[hash]=prevEnt;
    }else{
        prevEnt->chained=entPtr->chained;
        free(entPtr);
    }


}

void report() {
    relTOrder *iterator = orderQueue;
    int none = 1;
    for (; iterator != NULL; iterator = iterator->chained) {
        if (relTypeTable[iterator->arrayPos].needCorrection == 0) {
            if (relTypeTable[iterator->arrayPos].max > 0) {
                if (none == 0) {
                    printf(" ");
                }
                none = 0;
                printf("\"%s\" ", relTypeTable[iterator->arrayPos].name);
                maxEntity *maxIterator = relTypeTable[iterator->arrayPos].pointerToList;
                for (; maxIterator != NULL; maxIterator = maxIterator->chained) {
                    printf("\"%s\" ", maxIterator->ptrTo->name);
                }
                printf("%d;", relTypeTable[iterator->arrayPos].max);
            }
        } else {
            //se needCorrection
            relTypeTable[iterator->arrayPos].max=0;
            relTypeTable[iterator->arrayPos].needCorrection=0;
            while(relTypeTable[iterator->arrayPos].pointerToList!=NULL){ //cancello i massimi già esistenti
                maxEntity *temp=relTypeTable[iterator->arrayPos].pointerToList->chained;
                free(relTypeTable[iterator->arrayPos].pointerToList);
                relTypeTable[iterator->arrayPos].pointerToList = temp;
            }
            for(int i=0; i<ENTITYTABLELENGTH; i++) {
                entity *entIterator = entityTable[i];
                while (entIterator != NULL) {
                    if (entIterator->occ[iterator->arrayPos] > 0) {
                        if (entIterator->occ[iterator->arrayPos] >
                            relTypeTable[iterator->arrayPos].max) { //se l'elemento ha relazioni più del massimo attuale
                            while (relTypeTable[iterator->arrayPos].pointerToList !=
                                   NULL) { //cancello i massimi già esistenti
                                maxEntity *temp = relTypeTable[iterator->arrayPos].pointerToList->chained;
                                free(relTypeTable[iterator->arrayPos].pointerToList);
                                relTypeTable[iterator->arrayPos].pointerToList = temp;
                            }
                            relTypeTable[iterator->arrayPos].pointerToList = malloc(
                                    sizeof(maxEntity)); //aggiungo il nuovo massimo
                            relTypeTable[iterator->arrayPos].pointerToList->ptrTo = entIterator;
                            relTypeTable[iterator->arrayPos].pointerToList->chained = NULL;
                            relTypeTable[iterator->arrayPos].max = entIterator->occ[iterator->arrayPos];
                        } else if (entIterator->occ[iterator->arrayPos] ==
                                   relTypeTable[iterator->arrayPos].max) { //se L'elemento ha tante relazioni quanto il massimo
                            if (strcmp(entIterator->name, relTypeTable[iterator->arrayPos].pointerToList->ptrTo->name) <
                                0) {
                                maxEntity *next = relTypeTable[iterator->arrayPos].pointerToList;
                                relTypeTable[iterator->arrayPos].pointerToList = malloc(sizeof(maxEntity));
                                relTypeTable[iterator->arrayPos].pointerToList->chained = next;
                                relTypeTable[iterator->arrayPos].pointerToList->ptrTo = entIterator;
                            } else {
                                maxEntity *list = relTypeTable[iterator->arrayPos].pointerToList->chained;
                                maxEntity *prev = relTypeTable[iterator->arrayPos].pointerToList;
                                for (; list != NULL; list = list->chained) {
                                    if (strcmp(entIterator->name, list->ptrTo->name) < 0) {
                                        break;
                                    }
                                    prev = list;
                                }
                                prev->chained = malloc(sizeof(maxEntity));
                                prev = prev->chained;
                                prev->chained = list;
                                prev->ptrTo = entIterator;
                            }
                        }
                    }
                    entIterator=entIterator->chained;
                }
            }
            if (relTypeTable[iterator->arrayPos].max > 0) {
                if (none == 0) {
                    printf(" ");
                }
                none = 0;
                printf("\"%s\" ", relTypeTable[iterator->arrayPos].name);
                maxEntity *maxIterator = relTypeTable[iterator->arrayPos].pointerToList;
                for (; maxIterator != NULL; maxIterator = maxIterator->chained) {
                    printf("\"%s\" ", maxIterator->ptrTo->name);
                }
                printf("%d;", relTypeTable[iterator->arrayPos].max);
            }
        }
    }
        if (none == 1) {
            printf("none");
        }
        printf("\n");
}