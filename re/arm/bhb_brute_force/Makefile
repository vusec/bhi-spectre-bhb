all: main.o
main.o: src/main.c src/snippet.S
	$(CC_ANDROID) -c src/snippet.S -o snippet.o -I../../../common -D$(TARGET)
	$(CC_ANDROID) src/main.c -o main.o snippet.o -pie -O1 -I../../../common -D$(TARGET)
clean:
	rm -f *.o
