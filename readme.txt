Taylor Arnett & Hongyang Liu
CMPUT 391 Assignment 2


Q4
    to compile q4.c use the following command line instruction:
        ```
        gcc -g q4.c sqlite3.c -lpthread -ldl -DSQLITE_ENABLE_RTREE=1
        ```

    to execute use:
        ```
        ./a.out <database file> <x1> <y1> <x2> <y2> <POI class name>
        ```
     where x1 and y1 are the top left coordinates and x2, y2 are the bottom right coordinates of a rectangle


Q5
    to compile q5.c use the following command line instruction:
        ```
        gcc -g -std=c99 q5.c sqlite3.c -lpthread -ldl -DSQLITE_ENABLE_RTREE=1
        ```

    to execute use:
        ```
        ./a.out <database file> <side length>
        ```


Q7
    to compile q7.c use the following command line instruction:
        ```
        gcc -g -std=c99 q7.c sqlite3.c -lpthread -ldl -DSQLITE_ENABLE_RTREE=1
        ```

    to execute use:
        ```
        ./a.out <database file> x y
        ```
	where x,y are the coordinates of the point, we choose to use the minDist as the distance between a Point and an Object


Q8
    to compile q8.c use the following command line instruction:
        ```
        gcc -g -std=c99 q8.c sqlite3.c -lpthread -ldl -DSQLITE_ENABLE_RTREE=1
        ```

    to execute use:
        ```
        ./a.out <database file> x y k
        ```
	where x,y are the coordinates of the point, and k is the parameter to require the program return k nearest neightbors. we choose to use the minDist as the distance between a Point and an Object
