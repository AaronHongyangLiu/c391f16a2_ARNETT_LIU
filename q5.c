#define _GNU_SOURCE // for asprintf

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


int MAX_BOUNDING_SQUARES = 100; // from assignment request
int QUERY_RUNS = 20;

double time_query(sqlite3 **db_ptr, double min_Xi, double max_Xi, double min_Yi, double max_Yi, int Rtree);

int main(int argc, char **argv) {
    sqlite3 *db; //the database
    srand(time(
            NULL)); //for generating random numbers (from http://stackoverflow.com/questions/822323/how-to-generate-a-random-number-in-c)

    int rc;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <database file> <side length>\n", argv[0]);
        return (1);
    }

    // Set argument
    double side_length = atof(argv[2]);
    int side_length_int = atoi(argv[2]);
    if ((side_length <= 0.00) || (side_length > 1000.00)) {
        fprintf(stderr, "Side length must be between 0 and 1000. You entered %.4f.\n", side_length);
        return (1);
    }

    rc = sqlite3_open(argv[1], &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (1);
    }

    float total_std = 0, total_rtree = 0;

    // MAX_BOUNDING_SQUARES * 4 because there are 4 coordinates per square
    double bounding_squares_array[
            MAX_BOUNDING_SQUARES * 4]; // [min_X1, max_X1, min_Y1, max_Y1, min_X2, max_X2, min_Y2, max_Y2,...]

    for (int i = 0; i < MAX_BOUNDING_SQUARES * 4; i += 4) {
        // randomly generates the bottom left coordinate of the new box
        double x = rand() % (1000 - side_length_int + 1); // random integer between 0 - 1000 specifying X coord
        double y = rand() % (1000 - side_length_int + 1); // random integer between 0 - 1000 specifying Y coord

        bounding_squares_array[i] = x;
        bounding_squares_array[i + 1] = x + side_length;
        bounding_squares_array[i + 2] = y;
        bounding_squares_array[i + 3] = y + side_length;

    }
    double total_time_std = 0, total_time_rtree = 0;
    for (int i = 0; i < QUERY_RUNS; i++) {
        for (int j = 0; j < MAX_BOUNDING_SQUARES * 4; j += 4) {
            total_time_std += time_query(&db, bounding_squares_array[j], bounding_squares_array[j + 1],
                                         bounding_squares_array[j + 2], bounding_squares_array[j + 3], 0);
            total_time_rtree += time_query(&db, bounding_squares_array[j], bounding_squares_array[j + 1],
                                           bounding_squares_array[j + 2], bounding_squares_array[j + 3], 1);
        }
    }
    total_std += (total_time_std / 1000 / QUERY_RUNS);
    total_rtree += (total_time_rtree / 1000 / QUERY_RUNS);


    printf("Parameter l: %.4f \n"
                   "Average runtime with r-tree: %lf ms\n"
                   "Average runtime without r-tree: %lf ms\n", side_length, total_rtree / MAX_BOUNDING_SQUARES,
           total_std / MAX_BOUNDING_SQUARES);

    sqlite3_close(db);
}


double time_query(sqlite3 **db_ptr, double min_Xi, double max_Xi, double min_Yi, double max_Yi, int Rtree) {
    /**
     * times an sql query on a specified table in a given database
     *
     * param db_ptr: a reference to the database to query
     * param min_Xi: bottom left x coordinate
     * param max_Xi: top right x coordinate
     * param min_Yi: bottom left y coordinate
     * param max_Yi: top right y coordinate
     * param Rtree: a boolean specifying the table to run the query on
     */
    double time = 0;
    sqlite3_stmt *stmt; //the update statement
    char *min_X, *max_X, *min_Y, *max_Y;

    char *stmt_1;

    if (Rtree) {
        stmt_1 = (" SELECT COUNT(DISTINCT s.id)    \
                     FROM poi_index s \
                     WHERE s.minX > ");
    } else {
        stmt_1 = (" SELECT COUNT(DISTINCT s.id)    \
                    FROM std_index s \
                    WHERE s.minX > ");
    }
    char *stmt_2 = (" AND s.maxX < ");
    char *stmt_3 = (" AND s.minY > ");
    char *stmt_4 = (" AND s.maxY < ");
    char *stmt_5 = (";");

    asprintf(&min_X, "%.4f", min_Xi);
    asprintf(&max_X, "%.4f", max_Xi);
    asprintf(&min_Y, "%.4f", min_Yi);
    asprintf(&max_Y, "%.4f", max_Yi);

    // initialize the statement of appropriate length
    char sql_stmt[strlen(stmt_1) + strlen(stmt_2) + strlen(stmt_3) + \
                  strlen(stmt_4) + strlen(stmt_5) + strlen(min_X) + \
                  strlen(min_Y) + strlen(max_X) + strlen(max_Y)];

    strcpy(sql_stmt, stmt_1);
    strcat(sql_stmt, min_X);
    strcat(sql_stmt, stmt_2);
    strcat(sql_stmt, max_X);
    strcat(sql_stmt, stmt_3);
    strcat(sql_stmt, min_Y);
    strcat(sql_stmt, stmt_4);
    strcat(sql_stmt, max_Y);

    clock_t before, after, diff;

    before = clock();// start timer
    sqlite3_exec(*db_ptr, sql_stmt, NULL, NULL, NULL);
    after = clock(); // stop timer

    diff = (after - before);
    time = diff * 1000000 / CLOCKS_PER_SEC; // time in microseconds

    sqlite3_finalize(stmt);
    free(min_X);
    free(max_X);
    free(min_Y);
    free(max_Y);

    return time;

}
