CFLAGS = -I../../common -g -O3 -Wall -Wno-unused-function 

all: main.o

TARGET = undef
ifeq ($(TARGET),PIXEL_6)        
    CC := $(CC_ANDROID)
else
    CC := gcc
endif

main.o: src/main.c
	$(CC) -o $@ src/main.c $(CFLAGS) -D$(TARGET)

clean:
	rm -f *.o 
