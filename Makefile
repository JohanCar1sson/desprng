CC = gcc
CFLAGS = -O2 -ffast-math -finline-functions -funroll-loops -fomit-frame-pointer
CFLAGS = -g
CFLAGS =

FILES = desprng.h desprng.c toypicmcc.c crush1.c oldnewcomparison.c d3des.h d3des.c Makefile

.PHONY : all
all : libdesprng.a toypicmcc crush1 oldnewcomparison

libdesprng.a : desprng.o
	ar cr libdesprng.a desprng.o

desprng.o : desprng.c
	$(CC) $(CFLAGS) -c desprng.c

toypicmcc : toypicmcc.o libdesprng.a
	$(CC) -o toypicmcc toypicmcc.o -L. -ldesprng

toypicmcc.o : toypicmcc.c
	$(CC) $(CFLAGS) -c toypicmcc.c

crush1 : crush1.o libdesprng.a
	$(CC) -o crush1 crush1.o -L. -ldesprng -L$(HOME)/local/TestU01-1.2.3/lib64 -ltestu01 -lprobdist -lmylib -lgmp -lm -Wl,-rpath,$(HOME)/local/TestU01-1.2.3/lib64

crush1.o : crush1.c
	$(CC) $(CFLAGS) -I$(HOME)/local/TestU01-1.2.3/include -c crush1.c

oldnewcomparison : oldnewcomparison.o d3des.o libdesprng.a
	$(CC) -o oldnewcomparison oldnewcomparison.o d3des.o -L. -ldesprng

oldnewcomparison.o : oldnewcomparison.c
	$(CC) $(CFLAGS) -c oldnewcomparison.c

d3des.o : d3des.h d3des.c
	$(CC) $(CFLAGS) -c d3des.c

desprng.tgz : $(FILES)
	tar cfvz desprng.tgz $(FILES)

.PHONY : dist
dist : desprng.tgz

.PHONY : linecount
linecount :
	wc -l $(FILES)

.PHONY : clean
clean :
	rm -f libdesprng.a *.o toypicmcc crush1 oldnewcomparison d3des.out desprng.out *~ *.core
