CC = gcc
CFLAGS = -Wall -g -c
LD = gcc
LDFLAGS = -Wall -g

all: mytar

mytar: tools.o tarCreate.o tarExtract.o tarList.o mytar.o
	$(LD) $(LDFLAGS) -o mytar tools.o tarCreate.o tarExtract.o tarList.o mytar.o

tools.o: tools.c
	$(CC) $(CFLAGS) -o tools.o tools.c

tarCreate.o: tarCreate.c
	$(CC) $(CFLAGS) -o tarCreate.o tarCreate.c

tarExtract.o: tarExtract.c
	$(CC) $(CFLAGS) -o tarExtract.o tarExtract.c

tarList.o: tarList.c
	$(CC) $(CFLAGS) -o tarList.o tarList.c

mytar.o: mytar.c
	$(CC) $(CFLAGS) -o mytar.o mytar.c

clean:
	rm -f *.o
