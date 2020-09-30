#ifndef STRUTIL_H
#define STRUTIL_H

#define MARKER "#NIXADD#"
//#define MARKER "environment.systemPackages = with pkgs; [\n"

#include <stdio.h>
#include <string.h>

char *ltrim(char *);

int insertpkgs(char **, int, FILE *, FILE *);

#endif
