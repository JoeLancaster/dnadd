sources = nixadd.c
wflags = -Wall -Wextra -Wpedantic

default:
	gcc -O3 -Werror $(wflags) $(sources) -o nixadd
debug:
	gcc -g -O $(wflags) $(sources) -o nixadd_dbg

