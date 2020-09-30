#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <sys/wait.h>

#include "strutil.h"
#include "dynamic_read.h"
#include "rename.h"

//#define DEBUG

#define CMD "nixos-rebuild"
#define ARG "switch"

#define BS 256
#define RMAX (2 << 30)

#define DNA_SUFF "/.config/.nixadd"
#define CFG_DFLT "/etc/nixos/configuration.nix"
#define TMP_SUFF ".tmpnixadd"
#define OLD_SUFF ".nixadd"

const char *usage = "usage: %s [OPTIONS] ... [PKG] ...\n"
    "\t-c Specify nix configuration file for this use only\n"
    "\t-C Set persistent nix configuration file location\n"
    "\t-t Text only mode. Do not call `nixos-rebuild`\n"
    "\t-q Only print `nixos-rebuild` output if it fails\n" "\t-h Displays this message\n";

//extern char **environ;

int main(int argc, char **argv)
{
	int stat = 0;
	int quiet = 0;
	int io_p[2];
	int t_mode = 0;		//text only mode. do not nixos-rebuild
	int C_mode = 0;
	int c_sat = 0;
	int eno;
	int exit_usage = 0;
	int opt;
#ifdef DEBUG
	char *_config_path = "./configuration.nix";
	fprintf(stderr, "%s", "DEBUG BUILD\n");
#else
	char *_config_path = CFG_DFLT;	//default config location for NixOS
#endif
	char *C_str = NULL;
	while ((opt = getopt(argc, argv, "qtC:hc:")) != -1) {
		switch (opt) {
		case 'q':
			quiet = 1;
			break;
		case 't':
			t_mode = 1;
			break;
		case 'C':
			C_str = optarg;
			C_mode = 1;
			break;
		case 'c':
			c_sat = 1;
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

	char *home_path = getenv("HOME");
	char dna_path[strlen(home_path) + sizeof(DNA_SUFF)];
	strcpy(dna_path, home_path);
	strcat(dna_path, DNA_SUFF);

	if (C_mode) {

		errno = 0;
		char *C_path = realpath(C_str, NULL);	//let realpath alloc
		eno = errno;
		if (eno) {
			printf("Warning: %s didn't resolve correctly\n", C_str);
		} else {
			C_str = C_path;
		}
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
		free(C_path);
		return 0;
	}
	if (!c_sat) {
		char dna_cfg_path[PATH_MAX];	//the contents of ~/.config/.nixadd
		errno = 0;
		FILE *dna = fopen(dna_path, "r");
		eno = errno;
		if (eno && eno != ENOENT) {
			fprintf(stderr, "Error: couldn't open ~" DNA_SUFF ": %s\n", strerror(eno));
			//fail or continue with default?
			exit(EXIT_FAILURE);
		}
		if (eno == ENOENT) {	// if ~/.config/.nixadd doesn't exist then notify user instead of failing

			/*
			   could stat /etc/nixos/configuration.nix for existence and prompt user iff:
			   ~/.config/.nixadd does not exist and /etc/nixos/configuration.nix does not exist
			 */

			puts("Note: you haven't set a location for your config yet. Use -C");
			puts("Defaulting to: " CFG_DFLT);
		} else {
			if (fgets(dna_cfg_path, PATH_MAX, dna) == NULL) {	//get the first line of .nixadd only
				fprintf(stderr, "Error reading from %s\n", dna_path);
				exit(EXIT_FAILURE);
			}
			_config_path = dna_cfg_path;
		}

		if (dna) {
			fclose(dna);
		}
	}

	errno = 0;
	char *cfg_full_path = realpath(_config_path, NULL);	//let realpath alloc
	eno = errno;

	int enorp = eno;
	errno = 0;
	FILE *fp = fopen(cfg_full_path, "r");
	eno = errno;
	//use _config_path instead of path_buf in failure in case it was realpath that failed

	if (eno || enorp) {
		if (eno == EACCES || enorp == EACCES) {
			fprintf(stderr, "Access denied for %s. (Are you sudo?)\n", _config_path);
		} else {
			fprintf(stderr, "Error opening file %s\n", _config_path);
		}
		if (fp) {
			fclose(fp);
		}
		exit(EXIT_FAILURE);
	}

	const size_t cfp_len = strlen(cfg_full_path);

	char backup_file_path[cfp_len + sizeof(OLD_SUFF)];
	strcpy(backup_file_path, cfg_full_path);
	strcat(backup_file_path, OLD_SUFF);

	char temp_path[cfp_len + sizeof(TMP_SUFF)];
	strcpy(temp_path, cfg_full_path);
	strcat(temp_path, TMP_SUFF);

	errno = 0;
	FILE *dfp = fopen(backup_file_path, "w");
	eno = errno;
	if (eno) {
		fprintf(stderr, "Error with %s: %s\n", backup_file_path, strerror(eno));
		exit(EXIT_FAILURE);
	}

	insertpkgs(&argv[optind], argc - optind, fp, dfp);
	swap_names(backup_file_path, cfg_full_path, temp_path);
	fclose(fp);
	fclose(dfp);
	printf("Successfully edited %s\n", cfg_full_path);
	if (quiet) {		//ironically print more
		puts("Running " CMD "\n");	//so the user knows at least something's happening
	}
	if (!t_mode) {
		if (pipe(io_p) == -1) {
			exit(EXIT_FAILURE);
		}

		pid_t pid = fork();

		if (pid == -1) {
			perror("Fork:");
			exit(EXIT_FAILURE);
		} else if (pid > 0) {	//parent
			if (quiet) {
				close(io_p[1]);
				int bread = 0;
				int pf;
				char *buf = d_read(io_p[0], &bread, BS, RMAX, &pf);
				if (buf == NULL) {
				  fprintf(stderr, "Lost output from " CMD " " ARG "\n");
				}
				waitpid(pid, &stat, 0);
				if (stat) {
					printf("%.*s\n", bread, buf);
					free(buf);
				}
				if (pf) {
				  fprintf(stderr, "Lost some output from " CMD " " ARG "\n");
				}
			} else {
				waitpid(pid, &stat, 0);
			}
			puts("Done.\n");

		} else {
			if (quiet) {
				dup2(io_p[1], STDOUT_FILENO);
				dup2(io_p[1], STDERR_FILENO);
				close(io_p[0]);
				close(io_p[1]);
			}

			char *const _argv[] = { CMD, ARG, "--show-trace", NULL };

			if (execvpe(CMD, _argv, environ) < 0) {
				perror("execvpe:");
				exit(EXIT_FAILURE);
			}
		}
	}
	if (stat) {
	  fprintf(stderr, "%s: " CMD " failed.\n config has been reverted\n", argv[0]);
	  swap_names(backup_file_path, cfg_full_path, temp_path);
	}
	free(cfg_full_path);
	return stat;		//guaranteed 0 if (t) otherwise exit status of exec
}
