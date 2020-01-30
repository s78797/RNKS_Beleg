#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define BUFFER_LEN 256


char* getTxtColl(char* filesource) {

	FILE* fp;
	int linecount = 0;
	char buffer[BUFFER_LEN];

	fp = fopen(filesource, "r");
	if (fp == NULL)
	{
		exit(1);
	}

	if (fgets(buffer, BUFFER_LEN, fp) != NULL) {
		printf("gelesen: %s", buffer);

	}

	return buffer;
}

