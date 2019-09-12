//
// Created by alessandro on 12/09/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAMELENGTH 35
#define RELTYPENUMBER 50
#define ENTITYTABLELENGTH 500 //TODO
#define RELBLOCKLENGTH 500 //TODO
#define PROVISIONING 20 //entità e relazioni allocate per volta con malloc

typedef struct relatedEntity{  //Una relazione
    char entName[NAMELENGTH];
    struct relation *chained;
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
    //freopen("/home/alessandro/Scaricati/batch2.1.in","r",stdin);
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
addent(char *name){

}
addrel(char *from, char *to, char *relName){

}