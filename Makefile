CC = gcc
CFLAGS = -O2 -ffast-math -finline-functions -funroll-loops -fomit-frame-pointer
CFLAGS = -g
LDFLAGS =

CC = pgcc
CFLAGS = -acc -Minfo
LDFLAGS = -acc
CFLAGS = -ta=tesla:managed -Minfo
LDFLAGS = -ta=tesla:managed

FILES = desprng.h desprng.c toypicmcc.c oldnewcomparison.c d3des.h d3des.c Makefile crush0.c crush1.c crush2.c Makefile.crush

.PHONY : all
all : libdesprng.a toypicmcc

libdesprng.a : desprng.o
	ar cr libdesprng.a desprng.o

desprng.o : desprng.c
	$(CC) $(CFLAGS) -c desprng.c

toypicmcc : toypicmcc.o libdesprng.a
	$(CC) -o toypicmcc toypicmcc.o -L. -ldesprng $(LDFLAGS)

toypicmcc.o : toypicmcc.c
	$(CC) $(CFLAGS) -c toypicmcc.c

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
	rm -f libdesprng.a *.o toypicmcc oldnewcomparison d3des.out desprng.out *~ *.core
