#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>

//#define DEBUG

#define BUFLEN 255
#define MARKER "environment.systemPackages = with pkgs; [\n"
#define DNA_SUFF "/.config/.nixadd"

const char *usage = "usage: %s [OPTIONS] PKG\n"
    "\t-c Specify nix configuration file\n"
    "\t-C Set persistent nix configuration file location\n" "\t-h Displays this message\n";

char *ltrim(char *s)
{
	while (isspace(*s))
		s++;
	return s;
}

int main(int argc, char **argv)
{
	int C_mode = 0;
	int eno;
	int exit_usage = 0;
	int opt;
#ifdef DEBUG
	char *_config_path = "./configuration.nix";
	fprintf(stderr, "%s", "DEBUG BUILD\n");
#else
	char *_config_path = "/etc/nixos/configuration.nix";	//default config location for NixOS
#endif
	char *C_str = NULL;
	while ((opt = getopt(argc, argv, "C:hc:")) != -1) {
		switch (opt) {
		case 'C':
			C_str = optarg;
			C_mode = 1;
			break;
		case 'c':
			_config_path = optarg;
			break;
		case '?':
			exit(EXIT_FAILURE);	//getopt errors for us
			break;
		case 'h':
		default:
			exit_usage = 1;
		}
	}
	if (exit_usage || (optind == argc && !C_mode)) {
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	if (C_mode) {
		char C_path[PATH_MAX];
		errno = 0;
		realpath(C_str, C_path);
		eno = errno;
		if (eno) {
			printf("Warning: %s didn't resolve correctly\n", C_str);
		} else {
			C_str = C_path;
		}
		char *home_path = getenv("HOME");
		char dna_path[strlen(home_path) + sizeof(DNA_SUFF)];
		strcpy(dna_path, home_path);
		strcat(dna_path, DNA_SUFF);
		errno = 0;
		FILE *dna = fopen(dna_path, "w");
		eno = errno;
		if (eno) {
			fprintf(stderr, "Error opening %s\n", dna_path);
			fprintf(stderr, "%s", strerror(eno));
			exit(EXIT_FAILURE);
		}
		if (fputs(C_str, dna) < 0) {
			fprintf(stderr, "Couldn't write to %s\n", dna_path);
			fclose(dna);
			exit(EXIT_FAILURE);
		}
		fclose(dna);
		printf("Set %s as default config file.\n", C_str);
		return 0;
	}

	char *package = argv[optind];
	char *cfg_full_path;
	char *path_buf = malloc(PATH_MAX);
	char *path_cpy = malloc(PATH_MAX);
	char *path_tmp;
	char *fname_tmp;

	errno = 0;
	realpath(_config_path, path_buf);
	cfg_full_path = strdup(path_buf);
	eno = errno;
	int enorp = eno;
	errno = 0;
	FILE *fp = fopen(path_buf, "r");
	eno = errno;
	//use _config_path instead of path_buf in failure in case it was realpath that failed

	if (eno || enorp) {
		if (eno == EACCES || enorp == EACCES) {
			fprintf(stderr, "Access denied for %s. (Are you sudo?)\n", _config_path);
		} else {
			fprintf(stderr, "Error opening file %s\n", _config_path);
		}
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	strcpy(path_cpy, path_buf);

	char *cfg_file_name;
	char *cfg_dir;

	path_tmp = dirname(path_buf);	//[dir/base]name may modify its arg so we make use of the original (since it will be overwritten later) and the copy
	fname_tmp = basename(path_cpy);

	cfg_file_name = strdup(fname_tmp);	//strdup mallocs!
	cfg_dir = strdup(path_tmp);

	char *backup_file_path = malloc(PATH_MAX);	//the name of the backup file (to be created)
	strcpy(backup_file_path, cfg_full_path);
	strcat(backup_file_path, ".nixadd");

	char *temp_path = malloc(PATH_MAX);	//a temp name for swapping
	strcpy(temp_path, cfg_dir);
	strcat(temp_path, "/");
	strcat(temp_path, ".nixtemp");

	//we shouldn't need to use realpath for the backup file path.

	errno = 0;
	FILE *dfp = fopen(backup_file_path, "w");
	eno = errno;
	if (eno) {
		fprintf(stderr, "Error with %s: %s\n", backup_file_path, strerror(eno));
		exit(EXIT_FAILURE);
	}

	char buffer[BUFLEN];
	while (fgets(buffer, BUFLEN, fp)) {
		char *t = ltrim(buffer);
		fputs(buffer, dfp);
		if (strcmp(t, MARKER) == 0) {
			fprintf(dfp, "%s\n", package);
		}
	}

	//Swap files
	if (rename(cfg_full_path, temp_path) != 0) {
		fprintf(stderr, "can't rename configuration.nix");
		exit(EXIT_FAILURE);
	}
	if (rename(backup_file_path, cfg_full_path) != 0) {
		fprintf(stderr, "can't rename configuration.nix");
		exit(EXIT_FAILURE);
	}
	if (rename(temp_path, backup_file_path) != 0) {
		fprintf(stderr, "can't rename configuration.nix");
		exit(EXIT_FAILURE);
	}
	printf("Successfully edited %s\n", cfg_file_name);	//just to suppress warning for now
	free(temp_path);

	free(path_buf);
	free(path_cpy);
	fclose(fp);
	fclose(dfp);
	return 0;
}
