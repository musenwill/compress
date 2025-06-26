all:
	gcc -g -Wall -o compress *.c

clean:
	rm -rf compress
