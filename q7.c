#include "nearestNeighbor.h"

char *getQuery(char *xString, char *yString);

int pruneBranchList(struct Point p, int listLength, struct MBR *branchList, struct MBR *nearest, int afterRecursion);

void NNSearch(struct Node currentNode, struct Point p, struct MBR *nearest, int depth, int clevel);


int main(int argc, char **argv) {
    sqlite3_stmt *stmt; //the update statement
    int rc;

    // make sure we've got the argument for database
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <database file> x y\n", argv[0]);
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

    int valid_x = isNumber(argv[2], 0);
    int valid_y = isNumber(argv[3], 0);

    if ((!valid_x) || (!valid_y)) {
        fprintf(stderr, "x and y must both be numbers\n");
        return 1;
    }

    // open the database
    rc = sqlite3_open(argv[1], &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (1);
    }


    // create a function in sqlite3 that will return the nearest id of the neighbor
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
    if ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("ID: %10s | minX: %17s | maxX: %17s | minY: %17s | maxY: %17s \n",
               sqlite3_column_text(stmt, 0),
               sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3),
               sqlite3_column_text(stmt, 4)
        );
    }

    //finalize a statement
    sqlite3_finalize(stmt);
}


char *getQuery(char *xString, char *yString) {
    /**
     * function will reutn the query string
     * */
    char *q;
    float x, y;

    x = atof(xString);
    y = atof(yString);

    q = sqlite3_mprintf(
            "with nearest as                                                         \
             (select nnsearch(rtreenode(2,data), rtreedepth(data), %f, %f) as id     \
              from poi_index_node                                                    \
              where nodeno=1)                                                        \
             select i.* from poi_index i, nearest                                    \
             where i.id = nearest.id ;", x, y);
    return q;
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

    // update dist for the nearest neighbor with [strategy 2], for pruning in the child node
    // if dist(Object) > minMaxDist(MBR) || dist(MBR) > minMaxDist(MBR)
    if (!afterRecursion) {
        if (nearest->dist > minimum_minMaxDist) {
            nearest->dist = minimum_minMaxDist;
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

void NNSearch(struct Node currentNode, struct Point p, struct MBR *nearest, int depth, int clevel) {
    /**
     *
     * recursive function using the same algorithm given in the paper:
     * "N. Roussopoulos, S. Kelley, and F. Vincent. Nearest neighbor queries.
     * In Proceedings of the 1995 ACM SIGMOD International Conference on Management of Data,
     * San Jose, California, May 22-25, 1995., 71-79. ACM Press, 1995."
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
                // update nearest
                nearest->nodeno = currentMBR.nodeno;
                nearest->dist = currentMBR.dist;
                nearest->minDist = nearest->dist;
                nearest->minX = currentMBR.minX;
                nearest->maxX = currentMBR.maxX;
                nearest->minY = currentMBR.minY;
                nearest->maxY = currentMBR.maxY;
            }
            if (i != currentNode.count - 1) {  // update currentMBR if it's not the last mbr in the list
                currentMBR = *(currentMBR.next);
            }
        }

    } else {
        genBranchList(p, currentNode, &branchListHead);
        sortBranchList(&branchListHead, currentNode.count);
        count = pruneBranchList(p, currentNode.count, &branchListHead, nearest, 0);

        struct MBR current = branchListHead;
        // go through each MBR in the branchList
        for (int j = 0; j < count; j++) {
            newNode = getChildNode(current);   // get child node of the mbr
            NNSearch(newNode, p, nearest, depth, clevel + 1); // recursively calling NNSearch on child node
            count = pruneBranchList(p, count, &branchListHead, nearest, 1);

            freeList(newNode.MBRListHead, newNode.count);

            if (j != count - 1 && current.activeNext) {    // update current if it's not the last mbr in the active branch list
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
        // get all the parameters
        nodeString = (char *) sqlite3_value_text(argv[0]);
        depth = sqlite3_value_int(argv[1]);
        x = sqlite3_value_double(argv[2]);
        y = sqlite3_value_double(argv[3]);

        targetPoint.x = x;
        targetPoint.y = y;

        // initialize the nn
        nearestNeighbor.dist = MAX_DIST; // set this dist to be larger than the longest distance in a 1000*1000 grid.
        nearestNeighbor.nodeno = 0;

        rootNode.nodeNo = 1; // root node has nodeno = 1;
        buildNode(&nodeString, &rootNode); // build the count and mbr linked list

        NNSearch(rootNode, targetPoint, &nearestNeighbor, depth, 0);
        // return the id of the nearest neighbor id
        sqlite3_result_int64(context, nearestNeighbor.nodeno);
        freeList(rootNode.MBRListHead,rootNode.count);
    }
}

