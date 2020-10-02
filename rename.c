#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "rename.h"

void swap_names(char *dst, char *src, char *tmp)
{
	int eno;
	errno = 0;
	rename(src, tmp);
	eno = errno;
	if (eno) {
		puts(strerror(eno));
	}
	errno = 0;
	rename(dst, src);
	eno = errno;
	if (eno) {
		puts(strerror(eno));
	}
	errno = 0;
	rename(tmp, dst);
	eno = errno;
	if (eno) {
		puts(strerror(eno));
	}
}
