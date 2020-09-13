#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define BUFLEN 255
#define MARKER "environment.systemPackages = with pkgs; [\n"

char *ltrim(char *s)
{
	while (isspace(*s))
		s++;
	return s;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf
		    ("please specify the program name as the first argument\n");
		exit(EXIT_FAILURE);
	}
	char *package = argv[1];

	FILE *fp = fopen("./configuration.nix", "r");
	FILE *dfp = fopen("./configuration.nixadd-backup.nix", "w");
	if (fp == NULL || dfp == NULL)
		exit(EXIT_FAILURE);

	char buffer[BUFLEN];
	while (fgets(buffer, BUFLEN, fp)) {
		char *t = ltrim(buffer);
		fputs(buffer, dfp);
		if (strcmp(t, MARKER) == 0) {
			fprintf(dfp, "%s\n", package);
		}
	}
	// Swap files
	if (rename("./configuration.nix", "./.nixadd.tmp") != 0) {
		fprintf(stderr, "can't rename configuration.nix");
		exit(EXIT_FAILURE);
	}
	if (rename("./configuration.nixadd-backup.nix", "./configuration.nix") != 0) {
		fprintf(stderr, "can't rename configuration.nix");
		exit(EXIT_FAILURE);
	}
	if (rename("./.nixadd.tmp", "./configuration.nixadd-backup.nix") != 0) {
		fprintf(stderr, "can't rename configuration.nix");
		exit(EXIT_FAILURE);
	}
	fclose(fp);
	fclose(dfp);
	return 0;
}
