all:
	gcc -g -Wall -O0 -o compress *.c

clean:
	rm -rf compress
