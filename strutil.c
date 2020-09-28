#include "strutil.h"
#include <ctype.h>
#include <stdlib.h>

#define BUFLEN 256

char *ltrim(char *s)
{
	while (isspace(*s))
		s++;
	return s;
}

int countspaces(char *s)
{
	int i = 0;
	while (isspace(*s)) {
		s++;
		i++;
	}
	return i;
}

void insertpkgs(char *pkg, int plen, FILE * fp, FILE * dfp)
{
	char s[BUFLEN];
	int marked = 0;
	char *trimmed = malloc(BUFLEN);
	int i;
	while (fgets(s, BUFLEN, fp)) {
		i = 0;
		char *t = ltrim(s);
		printf("%s\n", t);
		while (!isspace(t[i]) && t[i] != '\0') {	//trim RHS
			trimmed[i] = t[i];
			i++;
		}
		trimmed[i] = '\0';
		fputs(s, dfp);
		if (strcmp(trimmed, MARKER) == 0 && !marked) {	//on the off chance user has marker twice
			char istr[BUFLEN];
			/*
			   make a copy of s which includes all the LHS white space
			   then cat pkg name from there, this handles user using mix of tabs and spaces
			 */
			strcpy(istr, s);
			i = 0;
			while (isspace(s[i])) {
				i++;
			}
			istr[i] = '\0';
			fprintf(dfp, "%s%s\n", istr, pkg);
			marked = 1;

		}
	}
	if (marked == 0) {
		fprintf(stderr, "Provided config file didn't contain \"" MARKER "\"\n");
		exit(EXIT_FAILURE);
	}
}
