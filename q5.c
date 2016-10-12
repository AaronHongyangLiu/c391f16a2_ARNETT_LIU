#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


int MAX_BOUNDING_SQUARES = 10; // from assignment request
int QUERY_RUNS = 20;

float std_time_query(sqlite3 **db_ptr, int min_X, int max_X, int min_Y, int max_Y);

int main(int argc, char **argv) {
    sqlite3 *db; //the database
    srand(time(
            NULL)); //for generating random numbers (from http://stackoverflow.com/questions/822323/how-to-generate-a-random-number-in-c)


    int rc;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <database file> <side length>\n", argv[0]);
        return (1);
    }

    rc = sqlite3_open(argv[1], &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (1);
    }

    // Set argument
    int side_length = atoi(argv[2]);

    for (int i = 0; i < MAX_BOUNDING_SQUARES; i++) {
        // randomly generates the bottom left coordinate of the new box
        int x = rand() % (1000 - side_length + 1); // random integer between 0 - 1000 specifying X coord
        int y = rand() % (1000 - side_length + 1); // random integer between 0 - 1000 specifying Y coord

        int minX = x;
        int maxX = x + side_length;
        int minY = y;
        int maxY = y + side_length;

        float total_time = 0;
        for (int j = 0; j < QUERY_RUNS; j++) {
            total_time += std_time_query(&db, minX, maxX, minY, maxY);
        }

        printf("[minX = %d, maxX = %d, minY = %d, maxY = %d]\n", minX, maxX, minY, maxY);
    }

    sqlite3_close(db);
}


float std_time_query(sqlite3 **db_ptr, int min_X, int max_X, int min_Y, int max_Y){
    float time = 0;
    int rc;
    sqlite3_stmt *stmt; //the update statement

    char *stmt_1 = (" SELECT DISTINCT s.id    \
                            FROM std_index s, \
                            WHERE s.minX > ");
    char *stmt_2 = (" AND s.maxX < ");
    char *stmt_3 = (" AND s.minY > ");
    char *stmt_4 = (" AND s.maxY < ");
    char *stmt_5 = (";");


    // initialize the statement of appropriate length
    char sql_stmt[strlen(stmt_1) + strlen(stmt_2) + strlen(stmt_3) + \
                  strlen(stmt_4) + strlen(stmt_5) + strlen(min_X) +  \
                  strlen(min_Y) + strlen(max_X) + strlen(max_Y)];

    strcpy(sql_stmt, stmt_1);
    strcat(sql_stmt, min_X);
    strcat(sql_stmt, stmt_2);
    strcat(sql_stmt, max_X);
    strcat(sql_stmt, stmt_3);
    strcat(sql_stmt, min_Y);
    strcat(sql_stmt, stmt_4);
    strcat(sql_stmt, max_Y);

    printf("%s\n", sql_stmt);

    clock_t before = clock(); // start timer
    rc = sqlite3_prepare_v2(*db_ptr, sql_stmt, -1, &stmt, 0);
    clock_t difference = clock() - before; // stop timer
    time = difference * 1000 / CLOCKS_PER_SEC;

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(*db_ptr));
        return 1;
    }

    sqlite3_finalize(stmt);

    return time; //TODO change to time variable

}

//    char * sql_stmt;
//
//
//
//

