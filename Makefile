MANDIR = /share/man/man1
sources = main.c strutil.c dynamic_read.c rename.c
CFLAGS = -Wall -Wextra -Wpedantic
PNAME = dnadd

default:
	gcc -O3 -Werror $(CFLAGS) $(sources) -o $(PNAME)
debug:
	gcc -g -Og $(CFLAGS) $(sources) -o $(PNAME)_dbg
install:
	install -d $(PREFIX)/bin
	install -m 777 $(PNAME) $(PREFIX)/bin
	install -d $(PREFIX)$(MANDIR)
	install -D $(PNAME).1 $(PREFIX)$(MANDIR)
	gzip -9 $(PREFIX)$(MANDIR)/$(PNAME).1
