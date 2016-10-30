
#ifndef C391F16A2_ARNETT_LIU_NEARESTNEIGHBOR_H
#define C391F16A2_ARNETT_LIU_NEARESTNEIGHBOR_H

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_DIST 2000000

sqlite3 *db; //the global database

struct MBR {
    long nodeno;
    double minX, maxX, minY, maxY, dist, minDist; // dist is minDist for an object and minMaxDist for an MBR
    struct MBR *next; // this is for the list of Node
    struct MBR *activeNext; // this is for the active branch list which is the sorted list
};

struct Point {
    double x, y;
};

struct Node {
    int nodeNo;
    int count; // total number of MBR
    struct MBR *MBRListHead; // a linked list of MBRs in this node
};

int maxNumberOfObject();

double minDist(struct MBR r, struct Point p);

double minMaxDist(struct MBR r, struct Point p);

void genBranchList(struct Point p, struct Node node, struct MBR *branchList);

void copy(struct MBR *src, struct MBR *dst);

void sortBranchList(struct MBR *branchList, int listLength);

struct Node getChildNode(struct MBR r);

void sqlite_nnsearch(sqlite3_context *context, int argc, sqlite3_value **argv);

void buildNode(char **ptrToString, struct Node *targetNodePtr);

int isNumber(char number[]);

void sqlite_nnsearch(sqlite3_context *context, int argc, sqlite3_value **argv);

void freeList(struct MBR *listHead, int listSize);


#endif //C391F16A2_ARNETT_LIU_NEARESTNEIGHBOR_H

