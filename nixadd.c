#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define BUFLEN 255
#define MARKER "environment.systemPackages = with pkgs; [\n"

const char *usage = "usage: %s PKG\n";

char *ltrim(char *s)
{
	while (isspace(*s))
		s++;
	return s;
}

int main(int argc, char **argv)
{
<<<<<<< HEAD
	int exit_usage = 0;
	int opt;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		case 'h':
		default:
			exit_usage = 1;
		}
	}
	if (exit_usage || optind == argc) {
		fprintf(stderr, usage, argv[0]);
=======
>>>>>>> 7fc689ceecfa4cc9011e2538aaa84e90d76ae26e
		exit(EXIT_FAILURE);
	}
	char *package = argv[optind];

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
	if (rename("./configuration.nixadd-backup.nix", "./configuration.nix")
	    != 0) {
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
