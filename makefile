CC = gcc 

ssi: ssi.c
	$(CC) ssi.c -o ssi  -lreadline

clean:
	rm -f ssi
