sources = nixadd.c
wflags = -Wall -Wextra -Wpedantic

debug:
	gcc -g -O $(wflags) $(sources) -o nixadd_dbg
default:
	gcc -O3 -Werror $(wflags) $(sources) -o nixadd
