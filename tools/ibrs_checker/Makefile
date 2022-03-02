CFLAGS = -I../../common -g -O3 -Wall -Wno-unused-function 

all: main.o

main.o: src/main.c
	gcc  -o $@ src/main.c $(CFLAGS) -DINTEL_10_GEN

clean:
	rm -f *.o 
