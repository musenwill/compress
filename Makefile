all:
	gcc -g -Wall -O0 -o compress *.c -lm

clean:
	rm -rf compress
