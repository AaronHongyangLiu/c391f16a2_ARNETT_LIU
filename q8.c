#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

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

char *getQuery(char *xString, char *yString, char *kString);

double minDist(struct MBR r, struct Point p);

double minMaxDist(struct MBR r, struct Point p);

void genBranchList(struct Point p, struct Node node, struct MBR *branchList);

void copy(struct MBR *src, struct MBR *dst);

void sortBranchList(struct MBR *branchList, int listLength);

int pruneBranchList(struct Point p, int listLength, struct MBR *branchList, struct MBR *nearest, int upward);

struct Node getChildNode(struct MBR r);

void NNSearch(struct Node currentNode, struct Point p, struct MBR *nearest, int depth, int clevel, int k);

void addObjectToList(struct MBR * object, struct MBR *listHead, int listSize);

void sqlite_nnsearch(sqlite3_context *context, int argc, sqlite3_value **argv);

void buildNode(char **ptrToString, struct Node *targetNodePtr);

int main(int argc, char **argv) {
    sqlite3_stmt *stmt; //the update statement
    int rc;

    // make sure we've got the argument for database
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <database file> x y k\n", argv[0]);
        return (1);
    }

    // open the database
    rc = sqlite3_open(argv[1], &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (1);
    }


    // create a function in sqlite3 that will return the nearest id of the neighbor
    sqlite3_create_function(db, "nnsearch", 5, SQLITE_UTF8, NULL, &sqlite_nnsearch, NULL, NULL);


    // get the query

    char *sql_stmt = getQuery(argv[2], argv[3], argv[4]);

    // execute the query
    rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }


    // display the query
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("%s", sqlite3_column_text(stmt, 0));
    }

    //finalize a statement
    sqlite3_finalize(stmt);
}

char *getQuery(char *xString, char *yString, char *kString) {
    /**
     * function will reutn the query string
     * */
    char *q;
    float x, y;
    int k;

    x = atof(xString);
    y = atof(yString);
    k = atoi(kString);

    q = sqlite3_mprintf(
            "select nnsearch(rtreenode(2,data), rtreedepth(data), %f, %f, %d)      \
             from poi_index_node                                                   \
             where nodeno=1;", x, y, k);
    return q;
}

double minDist(struct MBR r, struct Point p) {
    double rx, ry, result;

    // determine the value for rx
    if (p.x < r.minX) {
        rx = r.minX;
    } else if (p.x > r.maxX) {
        rx = r.maxX;
    } else {
        rx = p.x;
    }
    // determine the value for ry
    if (p.y < r.minY) {
        ry = r.minY;
    } else if (p.y > r.maxY) {
        ry = r.maxY;
    } else {
        ry = p.y;
    }

    // calculate the distance between point p and (rx,ry)
    result = (p.x - rx) * (p.x - rx) + (p.y - ry) * (p.y - ry);
    return result;

}

double minMaxDist(struct MBR r, struct Point p) {
    double rmx, rmy, rMx, rMy, dist1, dist2;

    // determine rmx
    if (p.x <= ((r.minX + r.maxX) / 2)) {
        rmx = r.minX;
    } else {
        rmx = r.maxX;
    }
    // determine rmy
    if (p.y <= ((r.minY + r.maxY) / 2)) {
        rmy = r.minY;
    } else {
        rmy = r.maxY;
    }

    // determine rMx
    if (p.x >= ((r.minX + r.maxX) / 2)) {
        rMx = r.minX;
    } else {
        rMx = r.maxX;
    }
    // determin rMy
    if (p.y >= ((r.minY + r.maxY) / 2)) {
        rMy = r.minY;
    } else {
        rMy = r.maxY;
    }

    // minMax = min( dist(p, (rmx, rMy)) and dist(p, (rMx, rmy)) )
    dist1 = (p.x - rmx) * (p.x - rmx) + (p.y - rMy) * (p.y - rMy);
    dist2 = (p.x - rMx) * (p.x - rMx) + (p.y - rmy) * (p.y - rmy);

    if (dist1 < dist2) {
        return dist1;
    }
    return dist2;
}

void genBranchList(struct Point p, struct Node node, struct MBR *branchList) {
    /**
     * function will iterate through all the MBRs in the node and assign each MBR its mindist and minmaxdist
     * then assign branchList to the head of this list
     */
    struct MBR *currentMBR = node.MBRListHead;
    for (int i = 0; i < node.count; i++) {
        currentMBR->dist = minMaxDist(*currentMBR, p);
        currentMBR->minDist = minDist(*currentMBR, p);
        if (i != node.count - 1) {
            currentMBR = currentMBR->next;
        }
    }
    *branchList = *(node.MBRListHead); //the branch list will point to the linked list of MBRs in the given node
}

void copy(struct MBR *src, struct MBR *dst) {
    /**
     * this function will copy data from MBR src to MBR dst.
     * */
    dst->nodeno = src->nodeno;
    dst->minX = src->minX;
    dst->minY = src->minY;
    dst->maxY = src->maxY;
    dst->maxX = src->maxX;
    dst->dist = src->dist;
    dst->minDist = src->minDist;
}

void sortBranchList(struct MBR *branchList, int listLength) {
    /**
     *
     * this function will sort the branchList based on the dist attribute of each MBR, using bubble sort
     *
     **/
    int i, j, k;
    struct MBR *current;
    struct MBR *next;
    struct MBR temp;

    int size = listLength;
    k = size;


    for (i = 0; i < size - 1; i++, k--) {
        current = branchList;      // go to the head of the list
        next = current->activeNext;

        for (j = 1; j < k; j++) {
            // swap the mbr
            if (current->dist > next->dist) {
                copy(current, &temp);
                copy(next, current);
                copy(&temp, next);
            }
            current = current->activeNext;
            next = current->activeNext;
        }

    }

}

int pruneBranchList(struct Point p, int listLength, struct MBR *branchList, struct MBR *nearest, int upward) {
    /**
     *
     * this function will prune the list and return the number of MBRs left in the pruned list
     *
     * */

    int length = listLength;
    struct MBR *previous = branchList;
    struct MBR *current = branchList->activeNext;
    double minimum_minMaxDist = branchList->dist;

    // update dist for the furthest nearest neighbor with [strategy 2], for pruning in the child node
    // if dist(Object) > minMaxDist(MBR) || dist(MBR) > minMaxDist(MBR)
    if (!upward) {
        if (nearest->dist < 2000000) {
            if (nearest->dist > minimum_minMaxDist) {
                nearest->dist = minimum_minMaxDist;
                nearest->nodeno = branchList->nodeno;
            }
        }
    }


    for (int i = 0; i < (listLength) - 1; i++) {
        // if minDist(MBR)> minDist(Object) || minDist(MBR) > minMaxDist(MBR)     [strategy 3 || 1 ]
        if ((current->minDist > nearest->dist) || (current->minDist > minimum_minMaxDist)) {
            previous->activeNext = current->activeNext;
            length -= 1;
        } else {
            previous = current;
        }

        if (i != (listLength) - 2) {
            current = current->activeNext;
        }
    }

    return length;
}

struct Node getChildNode(struct MBR r) {
    /***
     *
     * This function get all the child MBRs inside parent MBR r, return the result as a struct Node.
     *
     **/
    struct Node result;
    char *query, *queryResult;
    sqlite3_stmt *stmt;
    int rc;

    result.nodeNo = r.nodeno;
    query = sqlite3_mprintf("select rtreenode(2,data) \
                             from poi_index_node      \
                             where nodeno = %d; ", result.nodeNo);


    // execute the query
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }


    // get the query result
    rc = sqlite3_step(stmt);
    int col = 0;
    queryResult = (char *) sqlite3_column_text(stmt, col);


    // build up the node
    buildNode(&queryResult, &result);


    //finalize a statement, free the result of sqlite3_mprintf
    sqlite3_finalize(stmt);
    sqlite3_free(query);

    return result;
}

void addObjectToList(struct MBR * object, struct MBR *listHead, int listSize){
    /**
     *
     *  this function will add the object to the into the linked list and keep the list sorted
     * */
    struct MBR *previous, *current;
    struct MBR *copyOfObject; // to not mess up with a the node list, create a new object for the NNList

    copyOfObject = (struct MBR *)malloc(sizeof(struct MBR));
    copy(object,copyOfObject);

    if (listSize == 1){
        copy(copyOfObject,listHead);
        free(copyOfObject);
    } else {
        previous = listHead;
        current = listHead->next;
        for (int i=0; i<listSize-1; i++){
            if(copyOfObject->dist > current->dist){
                if (i==0){ // means the new object is the new list head
                    copy(copyOfObject,listHead);
                    free(copyOfObject);
                } else {
                    copyOfObject->next = current;
                    previous->next = copyOfObject;

                    struct MBR *newHead;
                    newHead = listHead->next;
                    copy(listHead,newHead);
                    listHead->next = newHead->next;
                    free(newHead);
                }
                break;
            }

            if (i == listSize-2){
                // means the new object should be at the end of the list
                current->next = copyOfObject;

                struct MBR *newHead;
                newHead = listHead->next;
                copy(listHead,newHead);
                listHead->next = newHead->next;
                free(newHead);
            } else {
                // if the for loop is not break, go to next neighbor in the list
                previous = previous->next;
                current = current->next;
            }
        }
    }


}

void NNSearch(struct Node currentNode, struct Point p, struct MBR *nearestListHead, int depth, int clevel, int k) {
    /**
     *
     * recursive function using the same algorithm given in the paper
     *
     * */

    struct Node newNode;
    struct MBR currentMBR;
    struct MBR branchListHead;
    int count;

    if (clevel == depth) {
        currentMBR = *(currentNode.MBRListHead);
        for (int i = 0; i < currentNode.count; i++) {
            // for an object in leaf-node, no need to store its minMaxDist
            currentMBR.dist = minDist(currentMBR, p);
            currentMBR.minDist = minDist(currentMBR, p);
            if (currentMBR.dist < nearestListHead->dist) {
                // update nearest
                addObjectToList(&currentMBR, nearestListHead, k);
            }
            if (i != currentNode.count - 1) {  // update currentMBR if it's not the last mbr in the list
                currentMBR = *(currentMBR.next);
            }
        }

    } else {
        genBranchList(p, currentNode, &branchListHead);
        sortBranchList(&branchListHead, currentNode.count);
        count = pruneBranchList(p, currentNode.count, &branchListHead, nearestListHead, 0);

        struct MBR current = branchListHead;
        // go through each MBR in the branchList
        for (int j = 0; j < count; j++) {
            newNode = getChildNode(current);   // get child node of the mbr
            NNSearch(newNode, p, nearestListHead, depth, clevel + 1, k); // recursively calling NNSearch on child node
            count = pruneBranchList(p, count, &branchListHead, nearestListHead, 1);

            if (j != count - 1) {    // update current if it's not the last mbr in the active branch list
                current = *(current.activeNext);
            }
        }
    }

}

void sqlite_nnsearch(sqlite3_context *context, int argc, sqlite3_value **argv) {
    /**
     * this will initialize the root node, and the linked list associate with it
     * then call the NNSearch function, and return the infomation of all k nearest neightbours
     *
     * */
    if (argc == 5) {

        char *nodeString, *resultString;
        double x, y;
        struct Node rootNode;
        struct Point targetPoint;
        struct MBR *nearestNeighborList, *current;
        int depth, k;
        // get all the parameters
        nodeString = (char *) sqlite3_value_text(argv[0]);
        depth = sqlite3_value_int(argv[1]);
        x = sqlite3_value_double(argv[2]);
        y = sqlite3_value_double(argv[3]);
        k = sqlite3_value_int(argv[4]);

        targetPoint.x = x;
        targetPoint.y = y;

        // initialize the nnList, we'll keep the list in descending order
        nearestNeighborList = (struct MBR *) malloc(sizeof(struct MBR));
        current = nearestNeighborList;
        for (int i = 0; i < k; i++) {
            current->dist = 2000000; // set this dist to be larger than the longest distance in a 1000*1000 grid.
            current->nodeno = 0;
            current->next = (struct MBR *) malloc(sizeof(struct MBR));

            // if current is the end of the list, go to next neighbor
            if (i != k - 1) {
                current = current->next;
            }
        }

        rootNode.nodeNo = 1; // root node has nodeno = 1;
        buildNode(&nodeString, &rootNode); // build the count and mbr linked list

        NNSearch(rootNode, targetPoint, nearestNeighborList, depth, 0, k);

        // form the output string
        resultString = (char *) malloc(0);
        current = nearestNeighborList;

        for (int j = 0; j < k; j++) {
            // form a newLine for the current neighbor

            char *newLine = (char *)sqlite3_mprintf("id: %ld | minX: %f | maxX: %f | minY: %f | maxY: %f | dist: %f\n",
                                            current->nodeno,
                                            current->minX, current->maxX,
                                            current->minY, current->maxY,
                                            current->dist
            );
            if(j!=0) {
                char *oldString = resultString;                         //copy the pointer of the old string
                size_t oldSize = strlen(oldString);                     // size of the oldString
                size_t increaseSize = strlen(newLine);                  // size of the newString

                resultString = (char *) malloc(oldSize + increaseSize + 1);  // +1 is for the NULL byte
                memcpy(resultString, oldString, oldSize);                 // copy the oldString to the resultString
                memcpy(resultString + oldSize, newLine, increaseSize + 1);    // append the newString to the end

                sqlite3_free(newLine);
                free(oldString);
            } else {
                resultString = (char *) malloc(strlen(newLine)+1);
                memcpy(resultString, newLine, strlen(newLine)+1);
                sqlite3_free(newLine);
            }

            // if current is the end of the list, go to next neighbor
            if (j != k - 1) {
                current = current->next;
            }
        }
        //printf("line 495\n%s",resultString);
        // return the output string
        sqlite3_result_text(context, resultString, (int) strlen(resultString), SQLITE_TRANSIENT);
        free(resultString);
    }
}

void buildNode(char **ptrToString, struct Node *targetNodePtr) {
    /**
     *
     *  this function build a node from string with format like:
     *  '{nodeno1 minX1 maxX1 minY1 maxY1} {nodeno2 minX2 maxX2 minY2 maxY2}'...
     *
     * */
    struct MBR *mbrPtr; // pointer to current mbr
    char *intString;    // a string that will contain an integer number
    int index;  // the index of the number
    // 0 means nodeno, 1 means minX, 2 means maxX, 3 means minY, 4 means maxY
    int inBracket = 0; // a flag about whether we are with in the curly bracket
    int stringLength;  // the value of strlen(intString)

    targetNodePtr->count = 0;
    // the for loop will go through each byte in the *ptrToString
    for (int i = 0; i < strlen(*ptrToString); i++) {
        switch ((*ptrToString)[i]) {
            case '{':
                inBracket = 1; // change the flag of whether we are inside a bracket to True
                index = 0;     // initialize the index for the this number
                targetNodePtr->count += 1; // increment the number of mbrs in the node
                stringLength = 0;          // initialize the strlen fo the variable intString
                intString = (char *) malloc(sizeof(char));
                intString = '\0';          // add the null byte at the end
                if (i == 0) {
                    // if this mbr is the head mbr in the list
                    mbrPtr = (struct MBR*)malloc(sizeof(struct MBR));       // create a mbr
                    targetNodePtr->MBRListHead = mbrPtr;       // add it to the head of the list
                } else {
                    mbrPtr->next = (struct MBR*)malloc(sizeof(struct MBR)); // add a new mbr to the end of the list
                    mbrPtr->activeNext = mbrPtr->next;         // add it to the active list as well
                    mbrPtr = mbrPtr->next;
                }
                break;

            case ' ':
                if (inBracket) {
                    switch (index) {
                        case 0: // if index == 0, assign it to nodeno
                            mbrPtr->nodeno = atol(intString);
                            break;
                        case 1: // if index == 1, assign it to minX
                            mbrPtr->minX = atof(intString);
                            break;
                        case 2: // if index == 2, assign it to maxX
                            mbrPtr->maxX = atof(intString);
                            break;
                        case 3: // if index == 3, assign it to minY
                            mbrPtr->minY = atof(intString);
                            break;
                    }
                    index += 1; // increment the index to the next number
                    free(intString); // free the pointer
                    intString = (char *) malloc(sizeof(char)); //get ready for next number
                    stringLength = 0; // initialize the strlen of intString
                }
                break;

            case '}':
                if (index == 4) { // if index == 4, assign it to maxY
                    mbrPtr->maxY = atof(intString);
                }

                inBracket = 0; // update the flag
                free(intString); // free the pointer
                break;

            default: // if this byte is part of a number
                intString = (char *) realloc(intString, (stringLength + 2)); // add 2 bytes for new char and \0
                intString[stringLength] = (*ptrToString)[i]; // add the char to the end of the string
                intString[stringLength + 1] = '\0';          // add the null terminate byte
                stringLength += 1;                           // update the strlen of intString

        }
    }

}
