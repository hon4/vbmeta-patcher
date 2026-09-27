#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define VBMETA_PATCHER_VER "0.0.3"

void show_help();
void show_ver();
void mkoutput_filename(const char *path, char **out);
int is_vbmeta(const char *file, size_t file_size);

int main(int argc, char *argv[]) {
	bool enable_verif = false; /* FALSE=Disable Verification, TRUE=Enable Verification */
	bool action_set = false;
	bool skip_magic_check = false; /* FALSE=Verify Magic, TRUE=Do not verify vbmeta magic */
	char *infile = NULL;
	char *outfile = NULL;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			show_help();
			return 0;
		} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
			show_ver();
			return 0;
		} else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--disable") == 0) {
			if (action_set) {
				fprintf(stderr, "Error: -e or --enable and -d or --disable cannot be used together.\n");
				return 1;
			}
			enable_verif = false;
			action_set = true;
		} else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--enable") == 0) {
			if (action_set) {
				fprintf(stderr, "Error: -e or --enable and -d or --disable cannot be used together.\n");
				return 1;
			}
			enable_verif = true;
			action_set = true;
		} else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
			i++; /* Next arg */
			outfile = argv[i]; /* gets the next of -o */
		} else if (strcmp(argv[i], "--skip-magic-check") == 0) {
			skip_magic_check = true;
		} else if (argv[i][0] == '-') {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			fprintf(stderr, "Type -h to show help.\n", argv[0]);
			return 1;
		} else {
			/* The input file */
			if (infile != NULL) {
				fprintf(stderr, "Error: only one input file is allowed.\n");
				return 1;
			}
			infile = argv[i];
		}
	}

	if (infile == NULL) {
		fprintf(stderr, "Error: No input file specified.\nType -h to show help.\n");
		return 1;
	}

	if (outfile == NULL) {
		mkoutput_filename(infile, &outfile);
	}

	/* Start the procedure */
	FILE *file = fopen(infile, "r");

	if (file == NULL) {
		perror("Error: Unable to read input file.\nfopen");
		return 1;
	}

	/* Find file size */
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);

	/* Allocate memory */
	char *content = malloc(size);
	if (content == NULL) {
		fclose(file);
		return 1;
	}

	/* Read file */
	size_t bytes_read = fread(content, 1, size, file);

	fclose(file);

	/* Check if file is a valid vbmeta.img (Has 'AVB0' magic at the begining) */
	if (!skip_magic_check && !is_vbmeta(content, bytes_read)) {
		fprintf(stderr, "Error: The selected image doesn't look like a valid vbmeta.img.\nError: Invalid magic.\n");
		return 1;
	}

	/* Check if file is enugh big */
	if (bytes_read < 123) {
		fprintf(stderr, "Error: The selected image is too small to be patched.\nIt should be > 123 bytes but it's %zu bytes.\n", bytes_read);
		return 1;
	}

	/* content now contains the vbmeta.img data - patch it */
	if (enable_verif) {
		content[123]=0x00; /* Enable Verification */
	} else {
		content[123]=0x03; /* Disable Verification */
	}

	/* Now write to outfile */
	FILE *foutfile = fopen(outfile, "wb");
	if (foutfile == NULL) {
		perror("Error: Unable to open output file.\nfopen");
		return 1;
	}

	size_t written = fwrite(content, 1, size, foutfile);

	fclose(foutfile);

	if (written != size) {
		printf("Error: Patching failed output filesize is different from source.\n");
		return 1;
	}

	free(content);
	
	printf("Success: Patching was successfull.\nVerification set to: %s\nOutput file: %s\n\n", (enable_verif ? "Enabled" : "Disabled"), outfile);
	return 0;
}

void show_help() {
	printf("vbmeta-patcher\n");
	printf("================\n");
	printf("\n");
	printf("Usage: vbmeta-patcher [OPTIONS] <input> [OPTIONS]\n");
	printf("\n");
	printf("Options:\n");
	printf("  -h, --help                Shows this help message and exit.\n");
	printf("  -v, --version             Shows this version info and exit.\n");
	printf("  -d, --disable             Set mode to disable verification.\n");
	printf("  -e, --enable              Set mode to enable verification.\n");
	printf("  -o, --output  <filename>  Specify the output filename.\n");
	printf("  --skip-magic-check        Skip the vbmeta magic verification.\n");
	printf("\n");
	printf("Notes:\n");
	printf("  1. You can't use enable and disable together.\n");
	printf("  2. If you don't specify output filename, the program will create a file with the same name as the source plus '-patched' at the end of the filename.\n");
	printf("\n");
}

void show_ver() {
	printf("vbmeta-patcher\n");
	printf("================\n");
	printf("Version: " VBMETA_PATCHER_VER "\n");
	printf("Language: C\n");
	printf("Coded by: hon\n");
	printf("\n");
}

void mkoutput_filename(const char *path, char **out) {
	const char *dot = strrchr(path, '.');

	size_t len = strlen(path) + 8;  /* "-patched" + '\0' */

	*out = malloc(len);
	if (*out == NULL)
		return;

	if (dot)
		sprintf(*out, "%.*s-patched%s", (int)(dot - path), path, dot);
	else
		sprintf(*out, "%s-patched", path);
}

int is_vbmeta(const char *file, size_t file_size) {
	if (file == NULL || file_size < 4)
		return 0;

	return memcmp(file, "AVB0", 4) == 0;
}
