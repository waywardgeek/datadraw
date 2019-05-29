CFLAGS=-Wall -g
LFLAGS=-lddutil-dbg -DDD_DEBUG
CC=gcc

json: jndatabase.c json.c parseJSON.c json.h value.c object.c array.c
	$(CC) $(CFLAGS) -o json json.c parseJSON.c jndatabase.c value.c object.c array.c $(LFLAGS)

jndatabase.c: jndatabase.h

jndatabase.h: JSON.dd
	datadraw JSON.dd

clean:
	rm -f json jndatabase.c jndatabase.h
