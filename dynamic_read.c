#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "dynamic_read.h"

#define rd(B) /*read() with errors*/					\
  do {									\
    errno = 0;								\
    nr = read(fd, buf + rsf, (B));					\
    eno = errno;							\
    if (eno) {								\
      if (eno == EINTR){continue;}					\
      else if (rsf > 0) { /*if read fails before we've read anything*/	\
	goto pf;          /*then hard fail*/				\
      }									\
      else {								\
	goto errb;							\
      }									\
    }									\
    rsf+=nr;								\
  } while(0)

#define resize(S) /*realloc() with err checking*/		\
  do {								\
    tmp = realloc(buf, (S));					\
    if (tmp == NULL) { goto pf; } /*same idea as above*/	\
    buf = tmp;		/*but we hard fail at initial malloc*/	\
  } while (0)

/*
 * read() chunk bytes at a time from a file descriptor into a dynamic buffer
 * returns NULL if nothing can be read
 * returns a buffer and sets pf if data has been read but later failed
 * returns a buffer and does not set pf if successful
 */
char *d_read(int fd, int *_read, const size_t chunk, const size_t max, int *pf)
{
	size_t size = chunk;
	size_t rsf = 0;
	size_t nr = 0;
	char *buf, *tmp;
	int eno;
	*pf = 0;

	if (chunk > max || chunk == 0) {
		goto err;
	}
	buf = malloc(size);
	if (buf == NULL) {
		goto err;
	}
	do {
		do {
			if (chunk + rsf > max) {	//we cannot read a full chunk
				rd(max - rsf);
				goto pf;
			}
			rd(chunk);
		} while (nr == chunk && rsf + chunk <= size);
		if (nr == 0) {	//finished reading
			*_read = rsf;	//success
			resize(rsf);	//shrink to fit
			return buf;
		}
		if (rsf + chunk > size) {	//avoid superfluous resizing when nr == 0 i.e when we are done
			size = size * 2;
			resize(size);
		}
	} while (nr != 0);
	abort();		//should never be reached
 pf:
	*_read = rsf;
	*pf = 1;
	return buf;
 errb:
	free(buf);
 err:
	return NULL;
}
