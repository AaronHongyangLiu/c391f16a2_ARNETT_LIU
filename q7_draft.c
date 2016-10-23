//
// Created by aaron on 10/15/16.
//

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

char *getQuery();// the function that return the query as string

double minDist(struct MBR r, struct Point p);

double minMaxDist(struct MBR r, struct Point p);

void genBranchList(struct Point p, struct Node node, struct MBR *branchList);

void copy(struct MBR *src, struct MBR *dst);

void sortBranchList(struct MBR *branchList, int listLength);

int pruneBranchList(struct Point p, int listLength, struct MBR *branchList, struct MBR *nearest);

struct Node getChildNode(struct MBR r);

void NNSearch(struct Node currentNode, struct Point p, struct MBR *nearest, int depth, int clevel);

void sqlite_nnsearch(sqlite3_context *context, int argc, sqlite3_value **argv);

void buildNode(char **ptrToString, struct Node *targetNodePtr);

int main(int argc, char **argv) {
    sqlite3_stmt *stmt; //the update statement
    int rc;

    // make sure we've got the argument for database
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <database file> x y\n", argv[0]);
        return (1);
    }

    // open the database
    rc = sqlite3_open(argv[1], &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (1);
    }


    // create a function in sqlite3 that will return the nearest neighbor
    sqlite3_create_function(db, "nnsearch", 4, SQLITE_UTF8, NULL, &sqlite_nnsearch, NULL, NULL);


    // get the query

    char *sql_stmt = getQuery(argv[2], argv[3]);

    // execute the query
    rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }


    // display the query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int col;
        for (col = 0; col < sqlite3_column_count(stmt) - 1; col++) {
            printf("%s|", sqlite3_column_text(stmt, col));
        }
        printf("%s", sqlite3_column_text(stmt, col));
        printf("\n");
    }

    //finalize a statement
    sqlite3_finalize(stmt);
}

char *getQuery(char *xString, char *yString) {
    char *q;
    int x, y;
    x = atoi(xString);
    y = atoi(yString);

    q = sqlite3_mprintf(
            "select nnsearch(rtreenode(2,data), rtreedepth(data), %d, %d) from poi_index_node where nodeno=1", x,
            y);// 		      --|
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
     * function will iterate through a Nodes MBRs and assign each MBR its mindist and minmaxdist
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
    dst->minX = src->nodeno;
    dst->minY = src->nodeno;
    dst->maxY = src->nodeno;
    dst->maxX = src->nodeno;
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
        current = branchList;
        next = current->activeNext;


        for (j = 1; j < k; j++) {

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

int pruneBranchList(struct Point p, int listLength, struct MBR *branchList, struct MBR *nearest) {
    /**
     *
     * this function will prune the list and return the number of MBRs left in the pruned list
     * NOTE: as the branchList will never contain an Object, we do not need to compare dist(Object) with minMaxDist(MBR) [strategy 2]
     * */

    int length = listLength;
    struct MBR *previous = branchList;
    struct MBR *current = branchList->activeNext;
    double minimum_minMaxDist = branchList->dist;

    /* this may not be needed
    // if dist(Object) > minMaxDist(MBR) || dist(MBR) > minMaxDist(MBR)  <- ?????
    if (nearest->dist > minimum_minMaxDist){
        *nearest = *branchList;
    }*/


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
    query = sqlite3_mprintf("select rtreenode(2,data) from poi_index_node where nodeno = %d", result.nodeNo);


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

void NNSearch(struct Node currentNode, struct Point p, struct MBR *nearest, int depth, int clevel) {
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
            if (currentMBR.dist < nearest->dist) {
                nearest->nodeno = currentMBR.nodeno;
                nearest->dist = currentMBR.dist;
                nearest->minDist = nearest->dist;
                nearest->minX = currentMBR.minX;
                nearest->maxX = currentMBR.maxX;
                nearest->minY = currentMBR.minY;
                nearest->maxY = currentMBR.maxY;
            }
            if (i != currentNode.count - 1) {
                currentMBR = *(currentMBR.next);
            }
        }

    } else {
        genBranchList(p, currentNode, &branchListHead);
        sortBranchList(&branchListHead, currentNode.count);
        count = pruneBranchList(p, currentNode.count, &branchListHead, nearest);

        struct MBR current = branchListHead;
        for (int j = 0; j < count; j++) {
            newNode = getChildNode(current);
            NNSearch(newNode, p, nearest, depth, clevel + 1);
            count = pruneBranchList(p, count, &branchListHead, nearest);

            // need to increment current here along the active branch list
            if (j != count - 1) {
                current = *(current.activeNext);
            }
        }
    }

}

void sqlite_nnsearch(sqlite3_context *context, int argc, sqlite3_value **argv) {
    /**
     * this will initialize the root node, and the linked list associate with it
     * then call the NNSearch function, and return the id of nearest neighbor
     *
     * */
    if (argc == 4) {

        char *nodeString;
        double x, y;
        struct Node rootNode;
        struct Point targetPoint;
        struct MBR nearestNeighbor;
        int depth;

        nodeString = (char *) sqlite3_value_text(argv[0]);
        depth = sqlite3_value_int(argv[1]);
        x = sqlite3_value_double(argv[2]);
        y = sqlite3_value_double(argv[3]);

        targetPoint.x = x;
        targetPoint.y = y;

        // initialize the nn
        nearestNeighbor.dist = 2000000; // set this dist to be larger than the longest distance in a 1000*1000 grid.
        nearestNeighbor.nodeno = 0;

        rootNode.nodeNo = 1; // root node has nodeno = 1;
        buildNode(&nodeString, &rootNode); // build the count and mbr linked list

        NNSearch(rootNode, targetPoint, &nearestNeighbor, depth, 0);
        // return the id of the nearest neighbor id
        //printf("line 401, nearestNeightbor=[%f, %f, %f, %f], dist=%f\n",nearestNeighbor.minX, nearestNeighbor.maxX, nearestNeighbor.minY, nearestNeighbor.maxY, nearestNeighbor.dist);
        sqlite3_result_int64(context, nearestNeighbor.nodeno);
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
    char *intString; // a string that will contain an int
    int index; // the index of the number
    // 0 means nodeno, 1 means minX, 2 means maxX, 3 means minY, 4 means maxY
    int inBracket = 0; // a flag about whether we are with in the curly bracket
    int stringLength; // the value of strlen(intString)

    targetNodePtr->count = 0;
    for (int i = 0; i < strlen(*ptrToString); i++) {
        switch ((*ptrToString)[i]) {
            case '{':
                inBracket = 1;
                index = 0;
                targetNodePtr->count += 1;
                stringLength = 0;
                intString = (char *) malloc(sizeof(char));   // the empty bite at the end of the string
                intString = '\0';                   // add the null byte at the end
                if (i == 0) {
                    // head mbr in the list
                    mbrPtr = malloc(sizeof(struct MBR));
                    targetNodePtr->MBRListHead = mbrPtr;

                } else {
                    mbrPtr->next = malloc(sizeof(struct MBR));
                    mbrPtr->activeNext = mbrPtr->next;
                    mbrPtr = mbrPtr->next;
                }
                break;

            case ' ':
                if (inBracket) {
                    switch (index) {
                        case 0:
                            mbrPtr->nodeno = atol(intString);
                            break;
                        case 1:
                            mbrPtr->minX = atof(intString);
                            break;
                        case 2:
                            mbrPtr->maxX = atof(intString);
                            break;
                        case 3:
                            mbrPtr->minY = atof(intString);
                            break;
                    }
                    index += 1;
                    free(intString);
                    intString = (char *) malloc(sizeof(char)); //get ready for next number
                    stringLength = 0;
                }
                break;

            case '}':
                if (index == 4) {
                    mbrPtr->maxY = atof(intString);
                }

                inBracket = 0;
                free(intString);
                break;

            default:
                intString = (char *) realloc(intString, (stringLength + 2));
                intString[stringLength] = (*ptrToString)[i];
                intString[stringLength + 1] = '\0';
                stringLength += 1;

        }
    }

}
