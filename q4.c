#include <stdio.h>
#include <sqlite3.h>
#include <string.h>

// TODO this will be used if the coordinate input has brackets
// int MAX_COORDINATE_LEN =  8; // Two brackets, a comma and two 3 digit coordinates = 8

int main(int argc, char **argv) {
    sqlite3 *db; //the database
    sqlite3_stmt *stmt; //the update statement

    int rc;

    if (argc != 7) {
        // TODO this will be used if the coordinate input has brackets
        // fprintf(stderr, "Usage: %s <database file> <(x1,y1)> <(x2,y2)> <POI class name>\n", argv[0]);
        fprintf(stderr, "Usage: %s <database file> <x1> <y1> <x2> <y2> <POI class name>\n", argv[0]);
        return (1);
    }

    rc = sqlite3_open(argv[1], &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (1);
    }

    // TODO this will be used if the coordinate input has brackets
    // Grab the top left coordinates as x1,y1 before splitting by the comma
    // char top_left[MAX_COORDINATE_LEN];
    // memset(top_left, '\0', MAX_COORDINATE_LEN); // set to null bytes
    // int top_left_len = strlen(argv[2]) - 2; // subtract 2 to get rid of closing bracket and null byte
    // strncpy(top_left,argv[2]+1, top_left_len); // add one to get rid of opening bracket
    // printf("%s\n", top_left);

//    DEBUGGING TODO remove before submission
//    printf("x1 = %s\n",argv[2]);
//    printf("y1 = %s\n",argv[3]);
//    printf("x2 = %s\n",argv[4]);
//    printf("y2 = %s\n",argv[5]);

    char *stmt_1 = (" SELECT DISTINCT t.id              \
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
                  strlen(argv[6]) + strlen(argv[2]) + strlen(argv[3]) + \
                  strlen(argv[4]) + strlen(argv[5])];

    strcpy(sql_stmt, stmt_1);
    strcat(sql_stmt, argv[6]);
    strcat(sql_stmt, stmt_2);
    strcat(sql_stmt, argv[2]);
    strcat(sql_stmt, stmt_3);
    strcat(sql_stmt, argv[4]);
    strcat(sql_stmt, stmt_4);
    strcat(sql_stmt, argv[5]);
    strcat(sql_stmt, stmt_5);
    strcat(sql_stmt, argv[3]);
    strcat(sql_stmt, stmt_6);

    rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    if ((rc = sqlite3_step(stmt)) != SQLITE_ROW) {
        printf("No results were found using the given dimensions and class name '%s'\n", argv[6]);
    }
    // this prints each row of the result
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("%s\n", sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
