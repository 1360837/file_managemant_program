#include "sys_project9.h"

void cat(char *name)
{
	FILE *fp = fopen(name, "r");
	if (fp == NULL)
	{
		perror("fopen");
		return;
	}

	char c;
	system("clear");

	while ((c = fgetc(fp)) != EOF)
	{
		fputc(c, stdout);
	}

	fclose(fp);
}
