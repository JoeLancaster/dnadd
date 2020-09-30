#include "strutil.h"
#include <ctype.h>
#include <stdlib.h>

#define BUFLEN 2048		//max line length

char *ltrim(char *s)
{
	while (isspace(*s))
		s++;
	return s;
}

int insertpkgs(char **pkg, int plen, FILE * fp, FILE * dfp)
{
	char s[BUFLEN];
	int marked = 0;
	char trimmed[BUFLEN];
	int i;
	while (fgets(s, BUFLEN, fp)) {
		i = 0;
		char *t = ltrim(s);
		while (!isspace(t[i]) && t[i] != '\0') {	//trim RHS
			trimmed[i] = t[i];
			i++;
		}
		trimmed[i] = '\0';
		fputs(s, dfp);
		if (strcmp(trimmed, MARKER) == 0 && !marked) {	//on the off chance user has marker twice
		  char istr[BUFLEN]; //make a copy of s' whitespace
			strcpy(istr, s);
			i = 0;
			while (isspace(s[i])) {
				i++;
			}
			istr[i] = '\0';
			for (i = 0; i < plen; i++) {
				fprintf(dfp, "%s%s\n", istr, pkg[i]);
			}
			marked = 1;

		}
	}
	if (marked == 0) {
		fprintf(stderr, "Provided config file didn't contain \"" MARKER "\"\n");
		return 1;
	}
	return 0;
}
