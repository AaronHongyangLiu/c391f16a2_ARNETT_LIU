mytest: test_linkedlist.o linkedlist.o
	gcc -Wall -std=c99 -o mytest test_linkedlist.o linkedlist.o

linkedlist.o: linkedlist.c linkedlist.h
	gcc -Wall -std=c99 -DMW_STDIO -c linkedlist.c

test_linkedlist.o: test_linkedlist.c 
	gcc -Wall -std=c99 -DMW_STDIO -c test_linkedlist.c

unittest:
	make --no-print-directory mytest
	./mytest

clean:
	-rm *.o mytest

tar:
	tar cvf ll_v2.tar test_linkedlist.c linkedlist.c linkedlist.h Makefile 
