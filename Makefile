MANDIR = /share/man/man1
sources = main.c strutil.c dynamic_read.c rename.c
CFLAGS = -Wall -Wextra -Wpedantic
PNAME = dnadd

default:
	gcc -O3 -Werror $(CFLAGS) $(sources) -o $(PNAME)
debug:
	gcc -g -Og $(CFLAGS) $(sources) -o $(PNAME)_dbg
install:
	install -d $(DESTDIR)/bin
	install -m 777 $(PNAME) $(DESTDIR)/bin
	install -d $(DESTDIR)$(MANDIR)
	install $(PNAME).1 $(DESTDIR)$(MANDIR)
	gzip -9 $(DESTDIR)$(MANDIR)/$(PNAME).1
