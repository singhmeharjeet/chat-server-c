CC = gcc
CFLAGS = -g -Wall -Werror
OBJS = list.o chat_room.o
PROGRAM = s-talk

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJS) -lpthread

list.o: list.c list2.h
	$(CC) $(CFLAGS) -c list2.c -o list.o

chat_room.o: chat_room.c
	$(CC) $(CFLAGS) -c chat_room.c -o chat_room.o

clean:
	rm -f $(OBJS) $(PROGRAM)
