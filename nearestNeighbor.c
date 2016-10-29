#include "nearestNeighbor.h"

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
                    mbrPtr = (struct MBR *) malloc(sizeof(struct MBR));       // create a mbr
                    targetNodePtr->MBRListHead = mbrPtr;       // add it to the head of the list
                } else {
                    mbrPtr->next = (struct MBR *) malloc(sizeof(struct MBR)); // add a new mbr to the end of the list
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


// validates input -- from http://stackoverflow.com/questions/29248585/c-checking-command-line-argument-is-integer-or-not
int isNumber(char number[]) {
    for (int i = 0; number[i] != 0; i++) {
        //if (number[i] > '9' || number[i] < '0')
        if (!isdigit(number[i]))
            return 0;
    }
    return 1;
}

int maxNumberOfObject() {
    /**
     * this func will return the maximum number of objects in the database
     * */

    sqlite3_stmt *stmt;
    int rc;
    int result = -1;

    char *query = "select count(id) from poi_index;";

    // execute the query
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }


    // display the query
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result = atoi(sqlite3_column_text(stmt, 0));
    }

    //finalize a statement
    sqlite3_finalize(stmt);

    return result;

}
