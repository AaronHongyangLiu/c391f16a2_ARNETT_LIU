#include "nearestNeighbor.h"

char *getQuery(char *xString, char *yString, char *kString);

int pruneBranchList(struct Point p, int listLength, struct MBR *branchList, struct MBR *nearest, int afterRecursion);

void NNSearch(struct Node currentNode, struct Point p, struct MBR *nearest, int depth, int clevel, int k);

void addObjectToList(struct MBR *object, struct MBR *listHead, int listSize);


int main(int argc, char **argv) {
    sqlite3_stmt *stmt; //the update statement
    int rc;

    // make sure we've got the argument for database
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <database file> x y k\n", argv[0]);
        return (1);
    }

    // input validation
    if ((atof(argv[2]) < 0) || (atof(argv[2]) > 1000)) {
        fprintf(stderr, "x must be between 0 and 1000 [You entered %s]\n", argv[2]);
        return 1;
    }
    if ((atof(argv[3]) < 0) || (atof(argv[3]) > 1000)) {
        fprintf(stderr, "y must be between 0 and 1000 [You entered %s]\n", argv[3]);
        return 1;
    }
    if (atoi(argv[4]) < 0) {
        fprintf(stderr, "k must be a positive integer [You entered %s]\n", argv[4]);
        return 1;
    }

    int valid_x = isNumber(argv[2]);
    int valid_y = isNumber(argv[3]);
    int valid_k = isNumber(argv[4]);

    if ((!valid_x) || (!valid_y) || (!valid_k)) {
        fprintf(stderr, "x, y and k must be numbers\n");
        return 1;
    }

    // open the database
    rc = sqlite3_open(argv[1], &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (1);
    }

    // test if k > number of objects we have in database
    int maxK = maxNumberOfObject();
    if (maxK < 0) {
        return 1;
    }
    if (atoi(argv[4]) > maxK) {
        fprintf(stderr, "k value is too large, we only have %d objects in the database [You entered %s]\n", maxK,
                argv[4]);
        return 1;
    };

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


void addObjectToList(struct MBR *object, struct MBR *listHead, int listSize) {
    /**
     *
     *  this function will add the object to the into the linked list and keep the list sorted
     * */
    struct MBR *previous, *current;
    struct MBR *copyOfObject; // to not mess up with a the node list, create a new object for the NNList

    copyOfObject = (struct MBR *) malloc(sizeof(struct MBR));
    copy(object, copyOfObject);

    if (listSize == 1) {
        copy(copyOfObject, listHead);
        free(copyOfObject);
    } else {
        previous = listHead;
        current = listHead->next;
        for (int i = 0; i < listSize - 1; i++) {
            if (copyOfObject->dist > current->dist) {
                if (i == 0) { // means the new object is the new list head
                    copy(copyOfObject, listHead);
                    free(copyOfObject);
                } else {
                    copyOfObject->next = current;
                    previous->next = copyOfObject;
                    // change the list head
                    struct MBR *newHead;
                    newHead = listHead->next;
                    copy(newHead, listHead);
                    listHead->next = newHead->next;
                    free(newHead);
                }
                break;
            }

            if (i == listSize - 2) {
                // means the new object should be at the end of the list
                current->next = copyOfObject;
                // change the list head
                struct MBR *newHead;
                newHead = listHead->next;
                copy(newHead, listHead);
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

            freeList(newNode.MBRListHead, newNode.count);

            if (j != count - 1 &&
                current.activeNext) {    // update current if it's not the last mbr in the active branch list
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
        current->next = NULL;
        for (int i = 0; i < k; i++) {
            current->dist = MAX_DIST; // set this dist to be larger than the longest distance in a 1000*1000 grid.
            current->nodeno = 0;
            current->next = (struct MBR *) malloc(sizeof(struct MBR));
            current->next ->next = NULL;
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

            char *newLine = (char *) sqlite3_mprintf(
                    "ID: %10ld | minX: %10f | maxX: %10f | minY: %10f | maxY: %10f | dist: %14f | line: %d\n",
                    current->nodeno,
                    current->minX, current->maxX,
                    current->minY, current->maxY,
                    current->dist, k - j
            );
            if (j != 0) {
                char *oldString = resultString;                         //copy the pointer of the old string
                size_t oldSize = strlen(oldString);                     // size of the oldString
                size_t increaseSize = strlen(newLine);                  // size of the newString

                resultString = (char *) malloc(oldSize + increaseSize + 1);  // +1 is for the NULL byte
                memcpy(resultString, newLine, increaseSize);                 // copy the oldString to the resultString
                memcpy(resultString + increaseSize, oldString, oldSize + 1);    // append the newString to the end

                sqlite3_free(newLine);
                free(oldString);
            } else {
                resultString = (char *) malloc(strlen(newLine) + 1);
                memcpy(resultString, newLine, strlen(newLine) + 1);
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
        freeList(rootNode.MBRListHead, rootNode.count);
        freeList(nearestNeighborList, k);
    }
}

int pruneBranchList(struct Point p, int listLength, struct MBR *branchList, struct MBR *nearest, int afterRecursion) {
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
    if (nearest->dist < MAX_DIST) {  // to make sure we have at least k objects in the list
        if (!afterRecursion) {
            if (nearest->dist > minimum_minMaxDist) {  // update the near last node
                nearest->dist = minimum_minMaxDist;
                nearest->nodeno = branchList->nodeno;
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
    }

    return length;
}
