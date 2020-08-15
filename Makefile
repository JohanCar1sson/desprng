CC = gcc
CFLAGS = -O2 -ffast-math -finline-functions -funroll-loops -fomit-frame-pointer
CFLAGS = -g
CFLAGS =

FILES = oldnewcomparison.c desprng.h desprng.c d3des.h d3des.c Makefile

.PHONY : all
all : oldnewcomparison libdesprng.a

libdesprng.a : desprng.o
	ar cr libdesprng.a desprng.o

desprng.o : desprng.c
	$(CC) $(CFLAGS) -c desprng.c

oldnewcomparison : oldnewcomparison.o d3des.o libdesprng.a
	$(CC) -o oldnewcomparison oldnewcomparison.o d3des.o -L. -ldesprng

oldnewcomparison.o : oldnewcomparison.c
	$(CC) $(CFLAGS) -c oldnewcomparison.c

d3des.o : d3des.h d3des.c
	$(CC) $(CFLAGS) -c d3des.c

CrushDesPrng1 : CrushDesPrng1.o desprng.o make_unique_des_key.o d3des.o
	$(CC) -o CrushDesPrng1 CrushDesPrng1.o desprng.o make_unique_des_key.o d3des.o -L$(HOME)/local/TestU01-1.2.3/lib64 -ltestu01 -lprobdist -lmylib -lgmp -lm -Wl,-rpath,$(HOME)/local/TestU01-1.2.3/lib64

CrushDesPrng1.o : CrushDesPrng1.c
	$(CC) $(CFLAGS) -I$(HOME)/local/TestU01-1.2.3/include -c CrushDesPrng1.c

print_bits.o : print_bits.c
	$(CC) $(CFLAGS) -c print_bits.c

desprng.tgz : $(FILES)
	tar cfvz desprng.tgz $(FILES)

.PHONY : dist
dist : desprng.tgz

.PHONY : linecount
linecount :
	wc -l $(FILES)

.PHONY : clean
clean :
	rm -f oldnewcomparison *.o d3des.out *~ *.core
