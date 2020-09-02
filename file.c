#include "file.h"

#include <stdio.h>

long get_file_size(char * filename)
{
	long length = 0;
	FILE *fp = NULL;

	fp = fopen(filename, "rb");
	if(fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
	}

	if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

	return length;
}
