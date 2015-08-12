all: copyit

copyit: myshell.c
	gcc -Wall myshell.c -o myshell

clean:
	rm -f *.o myshell
