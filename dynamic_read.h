#ifndef DYNAMIC_READ_H
#define DYNAMIC_READ_H

#include <stddef.h>

char *d_read(int fd, int *read, const size_t chunk, const size_t max, int *pf);

#endif
