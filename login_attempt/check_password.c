#include <stdio.h>
#include <string.h>
#include "check_password.h"

#define STRING_BUFFER 256

static const char *user_head = "user="; // the start of an entry in the configuration file
static const char *configuration_filename = "/var/usrpasswd.ini"; // location of the configuration file

// Function Prototypes
//
static int check_file_for_string(const char *string);

// Checks the entire file for the specified string
static int check_file_for_string(const char *string)
{
	FILE *file;
	char line_buffer[STRING_BUFFER];
	int line_length;

	// open up the file
	file = fopen(configuration_filename, "r");

	if(file == NULL)
	{
		// Couldn't find the file
		return FAILURE;
	}
	else
	{
		while(fgets(line_buffer, STRING_BUFFER, file) != NULL)
		{
			line_length = strlen(line_buffer);
			if(line_length >= 2)
			{
				// Knock off access restriction since we don't use it
				if(line_buffer[line_length - 2] == ':')
				{
					line_buffer[line_length - 2] = '\0';
				}
			}
			if(line_length >= 3)
			{
				// Knock off access restriction since we don't use it
				if(line_buffer[line_length - 3] == ':')
				{
					line_buffer[line_length - 3] = '\0';
				}
			}

			if(strcmp(string, line_buffer) == 0)
			{
				// Close up the file
				fclose(file);

				// Successfully found it
				return SUCCESS;
			}
		}
	}

	// Close up the file
	fclose(file);

	// Didn't find the file
	return FAILURE;
}

// check the configuration file to see if there is a "username"
// with the password "password"
int user_match(const char *username, const char *password)
{
	char string[STRING_BUFFER];
	const int check_length = strlen(user_head) + strlen(username) + strlen(password) + 3; // for the separators and null character

	if(check_length > STRING_BUFFER)
	{
		fprintf(stderr, "Username and password are too long");
		return FAILURE;
	}
	else
	{
		(void)sprintf(string, "%s%s:%s", user_head, username, password);

		return check_file_for_string(string);
	}
}
