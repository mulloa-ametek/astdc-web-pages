#include <stdio.h>
#include <stdlib.h>
#include "cgi-lib.h"

#define CGI_RECORD_BUFFER_SIZE 1024
#define TRUE 1
#define FALSE 0

static const char *debug = "/var/debug_cgi.txt";
static const char *content_length = "CONTENT_LENGTH";
static FILE *diagnostic;
// Function Prototypes
//
// Gets out the content length
static int get_content_length(void);
static int unencode_string(char *string);

static int unencode_string(char *string)
{
	char *current = string;
	char *scan = string;
	int encoded_char;

	while(*scan != '\0')
	{
		if(*scan == '+')
		{
			// CGI define + as a space
			scan++;
			*current++ = ' ';
		}
		else if(*scan == '%')
		{
			++scan;
			if(sscanf(scan, "%2x", &encoded_char) == 1)
			{	
				// read 2 characters (2 hex digits)
				scan += 2;
				*current++ = (char) encoded_char;
			}
			else
			{
				// Failed to parse correctly
				return FALSE;
			}
		}
		else
		{
			*current++ = *scan++;
		}
	}

	// Terminate the string
	*current = '\0';

	// successful conversion
	return TRUE;
}

static int get_content_length(void)
{
	int result;

	char *env = getenv(content_length);

	if((env == NULL) || (sscanf(env, "%d", &result) != 1))
	{
		// The environment variable does not exist
		result = 0;
	}

	return result;
}

int scan_input(CGI_RECORD *record_list, int size)
{
	int record_number;
	int bytes_read;
	const int cgi_size = get_content_length();
	char buffer[CGI_RECORD_BUFFER_SIZE];
	char *p;
	char *record_p;
	int done = FALSE;

	if(cgi_size <= 0)
	{
		// Something went wrong getting the record size
		return 0;
	}
	else
	{
		// read in the buffer
		bytes_read = fread(buffer, sizeof(char), cgi_size, stdin);

		if(bytes_read != cgi_size || cgi_size == CGI_RECORD_BUFFER_SIZE)
		{
			// Something went wrong reading in the cgi data
			return 0;
		}
		else
		{
			// Null terminate the string to make it easier to work with
			buffer[bytes_read] = '\0';
			p = buffer;
			for(record_number = 0; !done && *p != '\0' && record_number != size; record_number++)
			{
				// extract the field name
				record_p = record_list[record_number].field_name;
				while(*p != '\0' && *p != '=')
				{
					*record_p++ = *p++;
				}
				if(*p == '\0')
				{

					// This shouldn't happen
					return 0;
				}
				else
				{
					*record_p = '\0';
					++p;
				}

				// extract the cgi field value
				record_p = record_list[record_number].field_value;
				while(*p != '\0' && *p != '&')
				{
					*record_p++ = *p++;
				}


				*record_p = '\0';
				if((*p == '\0') && record_number != (size - 1))
				{
					done = TRUE;
				}
				else
				{
					++p;
				}
			}
		}
	}

	if(done)
	{
		size = record_number;
	}

	// Now unencode all the strings
	for(record_number = 0; record_number != size; record_number++)
	{
		if(!unencode_string(record_list[record_number].field_name) || !unencode_string(record_list[record_number].field_value))
		{
			return 0;
		}
	}

	return cgi_size;
}

char *get_record(CGI_RECORD *record, const char *field, int size)
{
	int i;

	for(i = 0; i != size; ++i)
	{
		if(strcmp(field, record[i].field_name) == 0)
		{
			return record[i].field_value;
		}
	}

	// Couldn't find the record
	return NULL;
}
