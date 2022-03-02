CFLAGS = -I../../common -g -O3 -Wall -Wno-unused-function 

all: main.o

TARGET = undef

main.o: src/main.c src/snippet.S
	nasm -f elf64 src/snippet.S -o snippet.o
	gcc  -o $@ src/main.c snippet.o $(CFLAGS) -D$(TARGET)

clean:
	rm -f *.o 
