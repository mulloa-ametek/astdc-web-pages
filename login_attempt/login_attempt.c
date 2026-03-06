#include "../cgi-lib/cgi-lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// We only need to get the username and the password
#define POST_ENTRIES 2
#define ERROR 0
#define SUCCESS 1

static CGI_RECORD cgi_data[POST_ENTRIES];

static int get_cgi_data(char **username, char **password); 

static int get_cgi_data(char **username, char **password)
{
	const char *username_field_name = "login_name";
	const char *password_field_name = "password";

	// Parse the cgi input
	if(scan_input(cgi_data, POST_ENTRIES) == 0)
	{
		return ERROR;
	}

	// read out the 2 cgi fields
	*username = get_record(cgi_data, username_field_name, POST_ENTRIES);
	*password = get_record(cgi_data, password_field_name, POST_ENTRIES);

	// final validity check
	if((*username != NULL) && (*password != NULL))
	{
		return SUCCESS;
	}
	else
	{
		return ERROR;
	}
}

int main(void)
{
	char *username;
	char *password;

	if((get_cgi_data(&username, &password) != 0) && (user_match(username, password) != 0))
	{
		// Correct username and password
		// Go to the configuration page
		execlp("./lxi_configuration", "lxi_configuration", "/configuration.xsl", (char *)0);
	}
	else
	{
		// Incorrect username and/or password
		// Go to the failed login page
		execlp("./lxi_configuration", "lxi_configuration", "/failed_login.xsl", (char *)0);
	}
}
