#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAMELENGTH 35
#define RELTYPENUMBER 50
#define ENTITYTABLELENGTH 500 //TODO
#define RELBLOCKLENGTH 500 //TODO
#define PROVISIONING 1 //entità e relazioni allocate per volta con malloc

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
        headOfEntRecycler=malloc(sizeof(entity)*PROVISIONING);
        for(int i = 0; i<PROVISIONING-1; i++){
            headOfEntRecycler[i].chained = &headOfEntRecycler[i+1];
        }
        headOfEntRecycler[PROVISIONING-1].chained = NULL;
    }
    entity *toReturn = headOfEntRecycler;
    toReturn->chained=NULL;
    toReturn->name[0]='\0';
    headOfEntRecycler = headOfEntRecycler->chained;
    return toReturn;
}

void putEnt(entity *ptr){
    free(ptr);
    headOfEntRecycler=NULL;
   /* ptr->chained=headOfEntRecycler;
    headOfEntRecycler=ptr;*/
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
    toReturn->chained=NULL;
    toReturn->entName[0]='\0';
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

unsigned int relTypePos(char *name){ //Done
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
            orderQueue->chained = NULL;
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
                    prev->arrayPos=hashValue;
                    return hashValue;
                }
                prev=iterator;
            }
            prev->chained = malloc(sizeof(relTOrder));
            prev=prev->chained;
            prev->chained=iterator;
            prev->arrayPos=hashValue;
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
                    prev->arrayPos=hashValue;
                    return hashValue;
                }
                prev=iterator;
            }
            prev->chained = malloc(sizeof(relTOrder));
            prev=prev->chained;
            prev->chained=iterator;
            prev->arrayPos=hashValue;
        }
        return hashValue;
    }
}

entity *findEnt(char *name){ //DONE
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
    else if(strcmp(fromPtr->outgoingTable[typePos][relPos].entName, to)==0){
        return;
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
        prev->chained=NULL;
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
    toPtr->occ[typePos]++;//aggiorno numero relazioni
    //Gestione max
    if(relTypeTable[typePos].needCorrection==0){ //se il massimo deve essere già aggiornato, non ha senso perdere tempo
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
        }else if(toPtr->occ[typePos]==relTypeTable[typePos].max){
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
    //cancello dalla tabella uscente
    if(strcmp(fromPtr->outgoingTable[typePos][relPos].entName, to)==0){
        if(fromPtr->outgoingTable[typePos][relPos].chained==NULL){  //Se non c'è la coda, elimino e basta
            fromPtr->outgoingTable[typePos][relPos].entName[0]='\0';
        }else {//se c'è la coda, shifto di uno
            relation *toBeDel=fromPtr->outgoingTable[typePos][relPos].chained;
            strcpy(fromPtr->outgoingTable[typePos][relPos].entName, toBeDel->entName);
            fromPtr->outgoingTable[typePos][relPos].chained=toBeDel->chained;
            putRel(toBeDel);
        }
        //cancello dalla tabella entrante
        if(strcmp(toPtr->incomingTable[typePos][relPos].entName, from)==0){
            if(toPtr->incomingTable[typePos][relPos].chained==NULL){
                toPtr->incomingTable[typePos][relPos].entName[0]='\0';
            }else{
                relation *toBeDel=toPtr->incomingTable[typePos][relPos].chained;
                strcpy(toPtr->incomingTable[typePos][relPos].entName, toBeDel->entName);
                toPtr->incomingTable[typePos][relPos].chained = toBeDel->chained;
                putRel(toBeDel);
            }
        }else{
            relation *iterator = toPtr->incomingTable[typePos][relPos].chained;
            relation *prev = &toPtr->incomingTable[typePos][relPos];
            for(;iterator!=NULL; iterator=iterator->chained){
                if(strcmp(iterator->entName, to)==0){
                    prev->chained=iterator->chained;
                    putRel(iterator);
                    break;
                }
                prev=iterator;
            }
        }
        //verifico i massimi
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
                        maxEntity *prev = relTypeTable[typePos].pointerToList;
                        for (; iterator != NULL; iterator = iterator->chained) {
                            if (iterator->ptrTo == toPtr) {
                                maxEntity *temp = iterator->chained;
                                free(iterator);
                                prev->chained = temp;
                                break;
                            }
                            prev = iterator;
                        }
                    }
                }
            }
        }
        toPtr->occ[typePos]--;
        return;
    }else{
        relation *iterator = fromPtr->outgoingTable[typePos][relPos].chained;
        relation *prev = &fromPtr->outgoingTable[typePos][relPos];
        for(;iterator!=NULL; iterator=iterator->chained){
            if(strcmp(iterator->entName, to)==0){
                prev->chained=iterator->chained;
                putRel(iterator);
                //cancello dalla tabella entrante
                if(strcmp(toPtr->incomingTable[typePos][relPos].entName, from)==0){
                    if(toPtr->incomingTable[typePos][relPos].chained==NULL){
                        toPtr->incomingTable[typePos][relPos].entName[0]='\0';
                    }else{
                        relation *toBeDel=toPtr->incomingTable[typePos][relPos].chained;
                        strcpy(toPtr->incomingTable[typePos][relPos].entName, toBeDel->entName);
                        toPtr->incomingTable[typePos][relPos].chained = toBeDel->chained;
                        putRel(toBeDel);
                    }
                }else{
                    relation *iteratorEnt = toPtr->incomingTable[typePos][relPos].chained;
                    relation *prevEnt= &toPtr->incomingTable[typePos][relPos];
                    for(;iteratorEnt!=NULL; iteratorEnt=iteratorEnt->chained){
                        if(strcmp(iteratorEnt->entName, to)==0){
                            prevEnt->chained=iteratorEnt->chained;
                            putRel(iteratorEnt);
                            break;
                        }
                        prevEnt=iteratorEnt;
                    }
                }
                //verifico i massimi
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
                                maxEntity *iteratorMax = relTypeTable[typePos].pointerToList->chained;
                                maxEntity *prevMax = relTypeTable[typePos].pointerToList;
                                for (; iteratorMax != NULL; iteratorMax = iteratorMax->chained) {
                                    if (iteratorMax->ptrTo == toPtr) {
                                        maxEntity *temp = iteratorMax->chained;
                                        free(iteratorMax);
                                        prevMax->chained = temp;
                                        break;
                                    }
                                    prevMax = iteratorMax;
                                }
                            }
                        }
                    }
                }
                toPtr->occ[typePos]--;
                return;
            }
            prev=iterator;
        }
    }
}

void addEnt(char *name){ //DOne
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
        prev=prev->chained;
        strcpy(prev->name, name);
        prev->chained=NULL;
        for(int i=0; i<RELTYPENUMBER; i++){ //TODO: ottimizzabile ciclando tra le reltype realmente esistenti
            prev->occ[i]=0;
            prev->incomingTable[i]=NULL;
            prev->outgoingTable[i]=NULL;
        }
        return;
    }
}

void delEnt(char *name){
   unsigned int entityPos = hashEntity(name);
   if(entityTable[entityPos].name[0]=='\0'){
       return;
   }
   if(strcmp(entityTable[entityPos].name, name)==0){ //se è nella tabella
       if(entityTable[entityPos].chained!=NULL) {
           for (int i = 0; i < RELTYPENUMBER; i++) {
               if (relTypeTable[i].max > 0) {
                   if (entityTable[entityPos].chained->occ[i] == relTypeTable[i].max) {
                       relTypeTable[i].needCorrection = 1;
                   }
               }
           }
       }
       for(relTOrder *reltypeOrd = orderQueue; reltypeOrd!=NULL; reltypeOrd=reltypeOrd->chained){
           if(entityTable[entityPos].occ[reltypeOrd->arrayPos]==relTypeTable[reltypeOrd->arrayPos].max){
               relTypeTable[reltypeOrd->arrayPos].needCorrection=1;
           }
           if(entityTable[entityPos].outgoingTable[reltypeOrd->arrayPos]!=NULL) {  //cancello le relazioni uscenti
               for (int hashRel = 0; hashRel < RELBLOCKLENGTH; hashRel++) {
                   if (entityTable[entityPos].outgoingTable[reltypeOrd->arrayPos][hashRel].entName[0] != '\0') {//se esiste una relazione uscente da eliminare
                       //elimino la prima
                       entity *firstIncomingEnt = findEnt(entityTable[entityPos].outgoingTable[reltypeOrd->arrayPos][hashRel].entName);
                       if (strcmp(firstIncomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].entName, entityTable[entityPos].name) == 0) {  //se la relazione da eliminare è all'inizio
                           if (firstIncomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].chained == NULL) {
                               firstIncomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].entName[0] = '\0';
                           } else {
                               relation *tBr = firstIncomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].chained;
                               strcpy(firstIncomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].entName,
                                      tBr->entName);
                               firstIncomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].chained = tBr->chained;
                               putRel(tBr);
                           }
                       } else {
                           relation *prev = &firstIncomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel];
                           relation *act = firstIncomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].chained;
                           for (; act != NULL; act = act->chained) {
                               if (strcmp(act->entName, entityTable[entityPos].name) == 0) {
                                   prev->chained = act->chained;
                                   putRel(act);
                                   break;
                               }
                               prev = act;
                           }
                       }
                       //TODO gestione dei massimi
                       if(relTypeTable[reltypeOrd->arrayPos].needCorrection==0){
                           if(relTypeTable[reltypeOrd->arrayPos].max==firstIncomingEnt->occ[reltypeOrd->arrayPos]){
                               if(relTypeTable[reltypeOrd->arrayPos].pointerToList->chained==NULL){
                                   relTypeTable[reltypeOrd->arrayPos].needCorrection=1;
                               }
                               else{
                                   if(relTypeTable[reltypeOrd->arrayPos].pointerToList->ptrTo == firstIncomingEnt){
                                       maxEntity *tbd = relTypeTable[reltypeOrd->arrayPos].pointerToList;
                                       relTypeTable[reltypeOrd->arrayPos].pointerToList= relTypeTable[reltypeOrd->arrayPos].pointerToList->chained;
                                       free(tbd);
                                   }else{
                                       maxEntity *iterator = relTypeTable[reltypeOrd->arrayPos].pointerToList->chained;
                                       maxEntity *prev = relTypeTable[reltypeOrd->arrayPos].pointerToList;
                                       for (; iterator != NULL; iterator = iterator->chained) {
                                           if (iterator->ptrTo == firstIncomingEnt) {
                                               maxEntity *temp = iterator->chained;
                                               free(iterator);
                                               prev->chained = temp;
                                               break;
                                           }
                                           prev = iterator;
                                       }
                                   }
                               }
                           }
                       }
                       firstIncomingEnt->occ[reltypeOrd->arrayPos]--;
                       //elimino le relazioni concatenate
                       relation *iterator = entityTable[entityPos].outgoingTable[reltypeOrd->arrayPos][hashRel].chained;
                       while (iterator != NULL) {
                           entity *incomingEnt = findEnt(iterator->entName);
                           if (strcmp(incomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].entName,
                                      entityTable[entityPos].name) == 0) { //se la relazione da eliminare è all'inizio
                               if (incomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].chained == NULL) {
                                   incomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].entName[0] = '\0';
                               } else {
                                   relation *tBr = incomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].chained;
                                   strcpy(incomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].entName,
                                          tBr->entName);
                                   incomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].chained = tBr->chained;
                                   putRel(tBr);
                               }
                           } else {
                               relation *prev = &incomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel];
                               relation *act = incomingEnt->incomingTable[reltypeOrd->arrayPos][hashRel].chained;
                               for (; act != NULL; act = act->chained) {
                                   if (strcmp(act->entName, entityTable[entityPos].name) == 0) {
                                       prev->chained = act->chained;
                                       putRel(act);
                                       break;
                                   }
                                   prev = act;
                               }
                           }
                           iterator = iterator->chained;
                           //TODO gestione dei massimi
                           if(relTypeTable[reltypeOrd->arrayPos].needCorrection==0){
                               if(relTypeTable[reltypeOrd->arrayPos].max==incomingEnt->occ[reltypeOrd->arrayPos]){
                                   if(relTypeTable[reltypeOrd->arrayPos].pointerToList->chained==NULL){
                                       relTypeTable[reltypeOrd->arrayPos].needCorrection=1;
                                   }
                                   else{
                                       if(relTypeTable[reltypeOrd->arrayPos].pointerToList->ptrTo == incomingEnt){
                                           maxEntity *tbd = relTypeTable[reltypeOrd->arrayPos].pointerToList;
                                           relTypeTable[reltypeOrd->arrayPos].pointerToList= relTypeTable[reltypeOrd->arrayPos].pointerToList->chained;
                                           free(tbd);
                                       }else{
                                           maxEntity *iteratorBis = relTypeTable[reltypeOrd->arrayPos].pointerToList->chained;
                                           maxEntity *prev = relTypeTable[reltypeOrd->arrayPos].pointerToList;
                                           for (; iterator != NULL; iterator = iterator->chained) {
                                               if (iteratorBis->ptrTo == incomingEnt) {
                                                   maxEntity *tempBis = iteratorBis->chained;
                                                   free(iterator);
                                                   prev->chained = tempBis;
                                                   break;
                                               }
                                               prev = iteratorBis;
                                           }
                                       }
                                   }
                               }
                           }
                           firstIncomingEnt->occ[reltypeOrd->arrayPos]--;
                       }
                   }
               }
               free(entityTable[entityPos].outgoingTable[reltypeOrd->arrayPos]);
               entityTable[entityPos].outgoingTable[reltypeOrd->arrayPos]=NULL;
           }//finito con le rel uscenti
           if(entityTable[entityPos].incomingTable[reltypeOrd->arrayPos]!=NULL) {  //cancello le relazioni entranti
               for (int hashRel = 0; hashRel < RELBLOCKLENGTH; hashRel++) {
                   if(entityTable[entityPos].incomingTable[reltypeOrd->arrayPos][hashRel].entName[0]!='\0'){
                       entity *firstOutgoingEnt = findEnt(entityTable[entityPos].incomingTable[reltypeOrd->arrayPos][hashRel].entName);
                       unsigned int relPos = hashRelPos(entityTable[entityPos].incomingTable[reltypeOrd->arrayPos][hashRel].entName, entityTable[entityPos].name, relTypeTable[reltypeOrd->arrayPos].name);
                       if(strcmp(firstOutgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPos].entName, entityTable[entityPos].name)==0){
                           if(firstOutgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPos].chained==NULL){
                               firstOutgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPos].entName[0]='\0';
                           }else{
                               relation *tBr = firstOutgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPos].chained;
                               strcpy(firstOutgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPos].entName, tBr->entName);
                               firstOutgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPos].chained= tBr->chained;
                               putRel(tBr);
                           }
                       }else {
                           relation *prev = &firstOutgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPos];
                           relation *act = firstOutgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPos].chained;
                           for (; act != NULL; act = act->chained) {
                               if (strcmp(act->entName, entityTable[entityPos].name) == 0) {
                                   prev->chained = act->chained;
                                   putRel(act);
                                   break;
                               }
                               prev = act;
                           }
                       }
                       //elimino le relazioni concatenate
                       relation *iterator = entityTable[entityPos].incomingTable[reltypeOrd->arrayPos][hashRel].chained;
                       while (iterator != NULL) {
                           entity *outgoingEnt = findEnt(iterator->entName);
                           unsigned int relPosIt = hashRelPos(outgoingEnt->name, entityTable[entityPos].name, relTypeTable[reltypeOrd->arrayPos].name);

                           if(strcmp(outgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPosIt].entName, entityTable[entityPos].name)==0){
                               if(outgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPosIt].chained==NULL){
                                   outgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPosIt].entName[0]='\0';
                               }else{
                                   relation *tBr = outgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPosIt].chained;
                                   strcpy(outgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPosIt].entName, tBr->entName);
                                   outgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPosIt].chained= tBr->chained;
                                   putRel(tBr);
                               }
                           }else {
                               relation *prev = &outgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPosIt];
                               relation *act = outgoingEnt->outgoingTable[reltypeOrd->arrayPos][relPosIt].chained;
                               for (; act != NULL; act = act->chained) {
                                   if (strcmp(act->entName, entityTable[entityPos].name) == 0) {
                                       prev->chained = act->chained;
                                       putRel(act);
                                       break;
                                   }
                                   prev = act;
                               }
                           }
                           iterator = iterator->chained;
                       }
                   }
               }
               free(entityTable[entityPos].incomingTable[reltypeOrd->arrayPos]);
               entityTable[entityPos].incomingTable[reltypeOrd->arrayPos]=NULL;
           }
       }
       if(entityTable[entityPos].chained==NULL){
           entityTable[entityPos].name[0]='\0';
       }else{
           entity *tbd = entityTable[entityPos].chained;
           strcpy(entityTable[entityPos].name, tbd->name);
           entityTable[entityPos].chained=tbd->chained;
           for(int i=0; i<RELTYPENUMBER ; i++){
               entityTable[entityPos].incomingTable[i]=tbd->incomingTable[i];
               entityTable[entityPos].outgoingTable[i]=tbd->outgoingTable[i];
               entityTable[entityPos].occ[i]=tbd->occ[i];
           }
           putEnt(tbd);
       }
   }else{//se è concatenato sotto
       entity *iteratorQue = entityTable[entityPos].chained;
       entity *prevQue = &entityTable[entityPos];
       for(; iteratorQue!=NULL; iteratorQue=iteratorQue->chained){
           if(strcmp(iteratorQue->name, name)==0){
               break;
               printf("\nstai provando a cancellare %s\n", name);
           }
           prevQue=iteratorQue;
       }
       if(iteratorQue==NULL){ //se sono uscito dall'if perchè non ho trovato l'entità, esco
           return;
       }
       /*

        */
   }
}

void report(){
    relTOrder *iterator = orderQueue;
    int none = 1;
    for(;iterator!=NULL; iterator=iterator->chained){
        if(relTypeTable[iterator->arrayPos].needCorrection==0){
            if(relTypeTable[iterator->arrayPos].max>0){
                if(none==0){
                    printf(" ");
                }
                none = 0;
                printf("\"%s\" ", relTypeTable[iterator->arrayPos].name);
                maxEntity *maxIterator = relTypeTable[iterator->arrayPos].pointerToList;
                for(;maxIterator!=NULL; maxIterator=maxIterator->chained){
                    printf("\"%s\" ", maxIterator->ptrTo->name);
                }
                printf("%d;", relTypeTable[iterator->arrayPos].max);
            }
        }else{
            //se needCorrection
            relTypeTable[iterator->arrayPos].max=0;
            relTypeTable[iterator->arrayPos].needCorrection=0;
            while(relTypeTable[iterator->arrayPos].pointerToList!=NULL){ //cancello i massimi già esistenti
                maxEntity *temp=relTypeTable[iterator->arrayPos].pointerToList->chained;
                free(relTypeTable[iterator->arrayPos].pointerToList);
                relTypeTable[iterator->arrayPos].pointerToList = temp;
            }
            for(int i=0; i<ENTITYTABLELENGTH; i++){
                if(entityTable[i].name[0]!='\0' && entityTable[i].occ[iterator->arrayPos]>0){  //se esiste l'elemento in tabella e ha relazioni
                    if(entityTable[i].occ[iterator->arrayPos]>relTypeTable[iterator->arrayPos].max){ //se l'elemento ha relazioni più del massimo attuale
                        while(relTypeTable[iterator->arrayPos].pointerToList!=NULL){ //cancello i massimi già esistenti
                            maxEntity *temp=relTypeTable[iterator->arrayPos].pointerToList->chained;
                            free(relTypeTable[iterator->arrayPos].pointerToList);
                            relTypeTable[iterator->arrayPos].pointerToList = temp;
                        }
                        relTypeTable[iterator->arrayPos].pointerToList=malloc(sizeof(maxEntity)); //aggiungo il nuovo massimo
                        relTypeTable[iterator->arrayPos].pointerToList->chained=NULL;
                        relTypeTable[iterator->arrayPos].pointerToList->ptrTo=&entityTable[i];
                        relTypeTable[iterator->arrayPos].max=entityTable[i].occ[iterator->arrayPos];
                    }else if(entityTable[i].occ[iterator->arrayPos]==relTypeTable[iterator->arrayPos].max){ //se L'elemento ha tante relazioni quanto il massimo
                        if(strcmp(entityTable[i].name, relTypeTable[iterator->arrayPos].pointerToList->ptrTo->name)<0){ //se l'elemento da aggiungere è minore del primo della lista
                            maxEntity *next = relTypeTable[iterator->arrayPos].pointerToList;
                            relTypeTable[iterator->arrayPos].pointerToList=malloc(sizeof(maxEntity));
                            relTypeTable[iterator->arrayPos].pointerToList->chained = next;
                            relTypeTable[iterator->arrayPos].pointerToList->ptrTo=&entityTable[i];
                        }
                        else {
                            maxEntity *list = relTypeTable[iterator->arrayPos].pointerToList->chained;
                            maxEntity *prev = relTypeTable[iterator->arrayPos].pointerToList;
                            for (; list != NULL; list = list->chained) {
                                if (strcmp(entityTable[i].name, list->ptrTo->name) < 0) {
                                    break;
                                }
                                prev = list;
                            }
                            prev->chained = malloc(sizeof(maxEntity));
                            prev = prev->chained;
                            prev->chained = list;
                            prev->ptrTo = &entityTable[i];
                        }
                    }
                }
                if(entityTable[i].chained!=NULL){ //se esistono seguiti
                    entity *entIterator = entityTable[i].chained;
                    for(; entIterator!=NULL; entIterator=entIterator->chained){
                        if(entIterator->occ[iterator->arrayPos]>0){
                            if(entIterator->occ[iterator->arrayPos]>relTypeTable[iterator->arrayPos].max){ //se l'elemento ha relazioni più del massimo attuale
                                while(relTypeTable[iterator->arrayPos].pointerToList!=NULL){ //cancello i massimi già esistenti
                                    maxEntity *temp=relTypeTable[iterator->arrayPos].pointerToList->chained;
                                    free(relTypeTable[iterator->arrayPos].pointerToList);
                                    relTypeTable[iterator->arrayPos].pointerToList = temp;
                                }
                                relTypeTable[iterator->arrayPos].pointerToList=malloc(sizeof(maxEntity)); //aggiungo il nuovo massimo
                                relTypeTable[iterator->arrayPos].pointerToList->ptrTo=entIterator;
                                relTypeTable[iterator->arrayPos].pointerToList->chained=NULL;
                                relTypeTable[iterator->arrayPos].max=entIterator->occ[iterator->arrayPos];
                            }else if(entIterator->occ[iterator->arrayPos]==relTypeTable[iterator->arrayPos].max){ //se L'elemento ha tante relazioni quanto il massimo
                                if(strcmp(entIterator->name, relTypeTable[iterator->arrayPos].pointerToList->ptrTo->name)<0){
                                    maxEntity *next = relTypeTable[iterator->arrayPos].pointerToList;
                                    relTypeTable[iterator->arrayPos].pointerToList=malloc(sizeof(maxEntity));
                                    relTypeTable[iterator->arrayPos].pointerToList->chained = next;
                                    relTypeTable[iterator->arrayPos].pointerToList->ptrTo=entIterator;
                                }else {
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
                    }
                }
            }//end for
            //Una volta ordinato stampo
            if(relTypeTable[iterator->arrayPos].max>0){
                if(none==0){
                    printf(" ");
                }
                none = 0;
                printf("\"%s\" ", relTypeTable[iterator->arrayPos].name);
                maxEntity *maxIterator = relTypeTable[iterator->arrayPos].pointerToList;
                for(;maxIterator!=NULL; maxIterator=maxIterator->chained){
                    printf("\"%s\" ", maxIterator->ptrTo->name);
                }
                printf("%d;", relTypeTable[iterator->arrayPos].max);
            }
        } //end else
    }
    if(none ==1){
        printf("none");
    }
    printf("\n");
}