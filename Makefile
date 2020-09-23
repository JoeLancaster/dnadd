MANDIR = /share/man/man1
sources = nixadd.c
CFLAGS = -Wall -Wextra -Wpedantic
PNAME = nixadd

default:
	gcc -O3 -Werror $(CFLAGS) $(sources) -o nixadd
debug:
	gcc -g -O $(CFLAGS) $(sources) -o nixadd_dbg
install:
	install -d $(DESTDIR)/bin
	install -m 777 $(PNAME) $(DESTDIR)/bin
	install -d $(DESTDIR)$(MANDIR)
	install $(PNAME).1 $(DESTDIR)$(MANDIR)
	gzip -9 $(DESTDIR)$(MANDIR)/$(PNAME).1
