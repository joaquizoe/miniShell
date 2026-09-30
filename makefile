CC = gcc 

ssi: ssi.c
	$(CC) ssi.c -lreadline -o ssi 

clean:
	rm -f ssi
