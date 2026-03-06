#include "../cgi-lib/cgi-lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

// Number of cgi entries will be greatest in the event that we have
// static configuration:
//
// 1) host_name
// 2) description
// 3) address_configuration = (static_ip | dhcp)
// 4) ip_address
// 5) subnet_mask
// 6) gateway
// 7) dns_server
#define POST_ENTRIES 1
#define ERROR 0
#define SUCCESS 1
#define TRUE 1
#define FALSE 0
#define QUERY_CHAR '?'

// location of the socket to execution pipe
#define SOCK_PATH "/var/tmp/web.socket"
// 100 character buffer to internal socket
#define BUFFER_SIZE 100

static CGI_RECORD cgi_data[POST_ENTRIES];

// Function prototypes
static int get_cgi_data(void);
static int form_socket(void);
static int transmit_data(int socket);

// CGI parameters
static char *command;

static int get_cgi_data(void)
{
	int entries_read;
	//  CGI variable names
	const char *command_name = "command";

	entries_read = scan_input(cgi_data, POST_ENTRIES);
	if(entries_read == 0)
	{
		// failed to read any values.  Something went wrong
		return ERROR;
	}
	// Parameters that are common to both DHCP and Static IP configurations
	command = get_record(cgi_data, command_name, POST_ENTRIES);
	if(command == NULL)
	{
		return -1;
	}
}

static int form_socket(void)
{
	int s, len;
	struct sockaddr_un remote;

	if((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		perror("socket call failed");
		exit(1);
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if(connect(s, (struct sockaddr *)&remote, len) == -1)
	{
		perror("connect call failed");
		exit(1);
	}

	// Fully connected
	// Return the socket
	return (s);
}

static int is_query(char *command)
{
	int i;
	int length;

	length = strlen(command);

	for(i = 0; i != length; ++i)
	{
		if(command[i] == QUERY_CHAR)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static int transmit_data(int socket)
{
	char str[BUFFER_SIZE];
	int query;
	int bytes_back;

	// data that needs to be sent regardless of the
	// configuration condition

	query = is_query(command);

	send(socket, command, strlen(command), 0);

	if(query)
	{
		if((bytes_back = recv(socket, str, BUFFER_SIZE, 0)) > 0)
		{
			str[bytes_back] = '\0';
			printf("Content-Type: text/plain\r\n\r\n%s", str);
		}
		else
		{
			printf("Content-Type: text/plain\r\n\r\nfailed_query");
		}
	}
	else
	{
		printf("Content-Type: text/plain\r\n\r\ndone");
	}

	return 0;
}

int main(void)
{
	int s; // The socket handle
	// Read out the CGI data
	get_cgi_data();

	// Open connection to the parser vis UNIX socket
	s = form_socket();
	// Send out the programming commands to the scpi parser
	transmit_data(s);

	usleep(10000);
	// Close the socket connection
	close(s);

	return 0;
}
