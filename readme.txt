CMPUT 391 Assignment 2

Taylor Arnett - 1364103
Hongyang Liu - 1449237


Q0

    first run the following command to generate tsv files before reading q0.txt in sqlite3
    '''
    python q0.py poi.tsv
    '''

Q1
    make sure you have ran `python q0.py poi.tsv` already before reading q1.txt in sqlite3


Before running programs q4-q8, make sure you have populated the database using `q0.txt` and `q1.txt`

Q4
    to compile q4.c use the following command line instruction:
        ```
        gcc -g q4.c sqlite3.c -lpthread -ldl -DSQLITE_ENABLE_RTREE=1
        ```

    to execute use:
        ```
        ./a.out <database file> <x1> <y1> <x2> <y2> <POI class name>
        ```
     Where x1 and y1 are the top left coordinates and x2, y2 are the bottom right coordinates of a rectangle.
     We assume that the objects must be fully contained by the bounding rectangle specs given.


Q5
    to compile q5.c use the following command line instruction:
        ```
        gcc -g -std=c99 q5.c sqlite3.c -lpthread -ldl -DSQLITE_ENABLE_RTREE=1
        ```

    to execute use:
        ```
        ./a.out <database file> <side length>
        ```

    In this question, we assume that the objects being found must be fully contained by the bounding rectangle.

Q7
    to compile q7.c use the following command line instruction:
        ```
        gcc -g -std=c99 q7.c sqlite3.c nearestNeighbor.c nearestNeighbor.h -lpthread -ldl -DSQLITE_ENABLE_RTREE=1
        ```

    to execute use:
        ```
        ./a.out <database file> x y
        ```
	where x,y are the coordinates of the point, we choose to use the minDist as the distance between a Point and an Object


Q8
    to compile q8.c use the following command line instruction:
        ```
        gcc -g -std=c99 q8.c sqlite3.c nearestNeighbor.c nearestNeighbor.h -lpthread -ldl -DSQLITE_ENABLE_RTREE=1
        ```

    to execute use:
        ```
        ./a.out <database file> x y k
        ```
	where x,y are the coordinates of the point, and k is the parameter to require the program return k nearest neighbors. 
    We choose to use the minDist as the distance between a Point and an Object
