#include "cgi-lib.h"
#include <stdio.h>

#define ARGUMENT_NUMBER 2

int main(void)
{
	int i;

	CGI_RECORD record[ARGUMENT_NUMBER];

	scan_input(record, ARGUMENT_NUMBER);
	
	for(i = 0; i != ARGUMENT_NUMBER; ++i)
	{
		printf("%s\n", get_record(record, record[i].field_name, ARGUMENT_NUMBER));
	}
	
	return 0;
}
