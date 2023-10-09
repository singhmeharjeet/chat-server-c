CC = gcc
CFLAGS = -g -Wall -Werror
OBJS = list.o chat_room.o
PROGRAM = chat_room

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJS)

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c -o list.o

test_cases.o: chat_room.c chat_room.h
	$(CC) $(CFLAGS) -c chat_room.c -o chat_room.o

clean:
	rm -f $(OBJS) $(PROGRAM)
