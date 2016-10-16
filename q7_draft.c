//
// Created by aaron on 10/15/16.
//

#include <stdio.h>
#include <sqlite3.h>

// the function that return the query as string
char *getQuery();

int main(int argc, char **argv){
    sqlite3 *db; //the database
    sqlite3_stmt *stmt; //the update statement
    int rc;

    // make sure we've got the argument for database
    if( argc!=2 ){
        fprintf(stderr, "Usage: %s <database file> \n", argv[0]);
        return(1);
    }

    // open the database
    rc = sqlite3_open(argv[1], &db);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }

    // get the query
    char *sql_stmt = getQuery();

    // execute the query
    rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // display the query
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int col;
        for(col=0; col<sqlite3_column_count(stmt)-1; col++) {
            printf("%s|", sqlite3_column_text(stmt, col));
        }
        printf("%s", sqlite3_column_text(stmt, col));
        printf("\n");
    }

    //finalize a statement
    sqlite3_finalize(stmt);
}

char *getQuery (){
    char * q;

    q = "select * from poi_index_node";// 		      --|
    return q;
}

struct MBR {
    int nodeno;
    double minX, maxX, minY, maxY, dist, mindist;
    struct MBR *next;
    struct MBR *activeNext;
};

struct Point {
    double x, y;
};

struct Node {
    int nodeno;
    int count; // total number of MBR
    struct MBR *MBRListHead; // a linked list of MBRs in this node
};

double minDist(struct MBR r, struct Point p){
    // TODO: return a mindist
}

double minMaxDist(struct MBR r, struct Point p){
    // TODO: return a minmaxdist
}

void genBranchList(struct Point p, struct Node n, struct MBR *branchList){
    // TODO
}

void sortBranchList(struct MBR *branchListHead){
    //TODO
}

int pruneBranchList(struct Point p, struct Node n, struct MBR *branchList, struct MBR *nearest){
    //TODO
    // return the number of element in the list
}

struct Node getChildNode(struct MBR){
    //TODO
    // return a node, by using sqlite....
}

void NNSearch(struct Node currentNode, struct Point p, struct MBR *nearest, int depth, int clevel){
    /*
     *
     * */

    struct Node newNode;
    struct MBR currentMBR;
    struct MBR branchListHead;
    int count;

    if (clevel==depth){
        currentMBR = *currentNode.MBRListHead;
        for (int i=0; i < currentNode.count; i++){
            currentMBR.dist = minDist(currentMBR,p);
            if (currentMBR.dist < (*nearest).dist){
                *nearest = currentMBR;
            }
            currentMBR = *(currentMBR.next);
        }
    } else {
        genBranchList(p,currentNode,&branchListHead);
        sortBranchList(&branchListHead);
        count = pruneBranchList(p,currentNode,&branchListHead,nearest);

        struct MBR current = branchListHead;
        for (int j=0; j<count; j++){
            newNode = getChildNode(current);
            NNSearch(newNode,p,&nearest,depth,clevel+1);
            count = pruneBranchList(p,currentNode,&branchListHead,nearest);
        }
    }

}
