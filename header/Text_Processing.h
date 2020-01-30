#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define BUFFER_LEN 256


FILE* get_file_pointer(char *filepath) {
	FILE *fp = fopen(filepath, "r");
	if (fp == NULL) {
		printf("Reading file from path '%s' failed with error: %d \n", filepath, errno);
		return -1;
	}
	return fp;
}


int write_to_file(char *filepath, char* buffer) {
	FILE *fp = fopen(filepath, "a");
	if (fp == NULL) {
		printf("Writing to file from path '%s' failed with error: %d \n", filepath, errno);
		return -1;
	}
	if (fputs(buffer, fp) == EOF) {
		printf("Reached EOF...\n");
		fclose(fp);
	}
}


int get_next_frame(char **buffer, FILE *fp) {
	if (fgets(*buffer, BUFFER_LEN, fp) != NULL) {
		return 1;
	}
	else {
		return -1;
	}
}

