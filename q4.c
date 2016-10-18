#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char **argv) {
    sqlite3 *db; //the database
    sqlite3_stmt *stmt; //the update statement

    int rc;

    if (argc != 7) {
        fprintf(stderr, "Usage: %s <database file> <x1> <y1> <x2> <y2> <POI class name>\n", argv[0]);
        return (1);
    }

    rc = sqlite3_open(argv[1], &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (1);
    }

    // Set arguments
    char *minX = argv[2];
    char *maxX = argv[4];
    char *minY = argv[5];
    char *maxY = argv[3];

    //error checking
    if (atoi(minX) >= atoi(maxX)) {
        fprintf(stderr, "x1 must be less than x2 [%s is not less than %s]\n", minX, maxX);
        return 1;
    }
    if (atoi(minY) >= atoi(maxY)) {
        fprintf(stderr, "y1 must be greater than y2 [%s is not greater than %s]\n", maxY, minY);
        return 1;
    }

    char *stmt_1 = (" SELECT DISTINCT t.id, i.minX, i.maxX, i.minY, i.MaxY, t.value \
                            FROM poi_tag t, poi_index i \
                            WHERE t.id = i.id AND       \
                                  t.key = 'class' AND   \
                                  t.value = '");
    char *stmt_2 = ("' AND i.minX > ");
    char *stmt_3 = (" AND i.maxX < ");
    char *stmt_4 = (" AND i.minY > ");
    char *stmt_5 = (" AND i.maxY < ");
    char *stmt_6 = (";");


    // initialize the statement of appropriate length
    char sql_stmt[strlen(stmt_1) + strlen(stmt_2) + strlen(stmt_3) + \
                  strlen(stmt_4) + strlen(stmt_5) + strlen(stmt_6) + \
                  strlen(argv[6]) + strlen(minX) + strlen(minY) + \
                  strlen(maxX) + strlen(maxY)];

    strcpy(sql_stmt, stmt_1);
    strcat(sql_stmt, argv[6]);
    strcat(sql_stmt, stmt_2);
    strcat(sql_stmt, minX);
    strcat(sql_stmt, stmt_3);
    strcat(sql_stmt, maxX);
    strcat(sql_stmt, stmt_4);
    strcat(sql_stmt, minY);
    strcat(sql_stmt, stmt_5);
    strcat(sql_stmt, maxY);
    strcat(sql_stmt, stmt_6);

    rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    int count = 0;
    // this prints each row of the result
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        count++; // counts number of results
        printf("ID: %10s | minX: %17s | maxX: %17s | minY: %17s | maxY: %17s | value: %s\n",
               sqlite3_column_text(stmt, 0),
               sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3),
               sqlite3_column_text(stmt, 4), sqlite3_column_text(stmt, 5)
        );
    }
    if (count == 0) {
        printf("No results were found using the given dimensions and class name '%s'\n", argv[6]);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
