all:
	gcc -g -Wall -Wextra visualize.c -o visualize -lgd -lfann
clean:
	rm visualize
