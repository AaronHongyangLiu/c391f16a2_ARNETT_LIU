//
// Created by aaron on 10/15/16.
//

#include <stdio.h>
#include <sqlite3.h>

// the function that return the query as string
char *getQuery();

int main(int argc, char **argv) {
    sqlite3 *db; //the database
    sqlite3_stmt *stmt; //the update statement
    int rc;

    // make sure we've got the argument for database
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <database file> \n", argv[0]);
        return (1);
    }

    // open the database
    rc = sqlite3_open(argv[1], &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (1);
    }

    // get the query
    char *sql_stmt = getQuery();

    // create a function in sqlite3 that will return the nearest neighbor
    sqlite3_create_function(db, "nnsearch", 4, SQLITE_UTF8, NULL, &sqlite_nnsearch, NULL, NULL);

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

char *getQuery() {
    char *q;


    q = "select nnsearch(rtreenode(2,data), rtreedepth(data) , %d, %d) from poi_index_node where nodeno=1";// 		      --|
    return q;
}

struct MBR {
    int nodeno;
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

void genBranchList(struct Point p, struct Node n, struct MBR *branchList) {
    /**
     * function will iterate through a Nodes MBRs and assign each MBR its mindist and minmaxdist
     */
    *branchList = *node.MBRListHead; //the branch list will point to the linked list of MBRs in the given node
    struct MBR *currentMBR = node.MBRListHead;
    while (*currentMBR != NULL) {
        currentMBR->dist = minMaxDist(*currentMBR, p);
        *currentMBR = currentMBR->next;
    }
}

void sortBranchList(struct MBR *branchList) {

    //TODO
}

int pruneBranchList(struct Point p, struct Node n, struct MBR *branchList, struct MBR *nearest) {
    //TODO
    // return the number of element in the list
}

struct Node getChildNode(struct MBR) {
    //TODO
    // return a node, by using sqlite....
}

void NNSearch(struct Node currentNode, struct Point p, struct MBR *nearest, int depth, int clevel) {
    /*
     * recursive function using the same algorithm given in the paper
     *
     * */

    struct Node newNode;
    struct MBR currentMBR;
    struct MBR branchListHead;
    int count;

    if (clevel == depth) {
        currentMBR = *currentNode.MBRListHead;
        for (int i = 0; i < currentNode.count; i++) {
            currentMBR.dist = minDist(currentMBR, p);
            if (currentMBR.dist < (*nearest).dist) {
                *nearest = currentMBR;
            }
            currentMBR = *(currentMBR.next);
        }
    } else {
        genBranchList(p, currentNode, &branchListHead);
        sortBranchList(&branchListHead);
        count = pruneBranchList(p, currentNode, &branchListHead, nearest);

        struct MBR current = branchListHead;
        for (int j = 0; j < count; j++) {
            newNode = getChildNode(current);
            NNSearch(newNode, p, &nearest, depth, clevel + 1);
            count = pruneBranchList(p, currentNode, &branchListHead, nearest);
        }
    }

}

void sqlite_nnsearch(sqlite3_context *context, int argc, sqlite3_value **argv) {
    /*
     * this will initialize the root node, and the linked list associate with it
     * then call the NNSearch function
     *
     * */
    if (argc == 4) {
        char *nodeString;
        double x, y;
        struct Node rootNode;
        struct Point targetPoint;
        struct MBR initialNearest;
        int depth;

        nodeString = sqlite3_value_text(argv[0]);
        depth = sqlite_value_int(argv[1]);
        x = sqlite3_value_double(argv[2]);
        y = sqlite3_value_double(argv[3]);

        targetPoint.x = x;
        targetPoint.y = y;

        initialNearest.dist = 10000; // set this dist to be larger than the longest distance in a 1000*1000 grid.

        buildNode(&nodeString, &rootNode);
        NNSearch(rootNode, targetPoint,initialNearest,depth, 0);
    }
}

void buildNode(char **ptrToString, struct Node *targetNode){
    /*
     *
     *  this function build a node from string with format like:
     *  '{nodeno1 minX1 maxX1 minY1 maxY1} {nodeno2 minX2 maxX2 minY2 maxY2}'...
     *
     * */
}
