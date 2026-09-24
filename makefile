CC = gcc 
CFLAGS = -Wall -std=c11

ssi: ssi.c
	$(CC) $(CFLAGS) -o ssi ssi.c -lreadline

clean:
	rm -f ssi
