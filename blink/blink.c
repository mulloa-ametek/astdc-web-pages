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
static char *blink;

static int get_cgi_data(void)
{
	int entries_read;
	//  CGI variable names
	const char *blink_name = "blink";

	entries_read = scan_input(cgi_data, POST_ENTRIES);
	if(entries_read == 0)
	{
		// failed to read any values.  Something went wrong
		return ERROR;
	}
	// Parameters that are common to both DHCP and Static IP configurations
	blink = get_record(cgi_data, blink_name, POST_ENTRIES);
	if(blink == NULL)
	{
		return -1;
	}
}

static int form_socket(void)
{
	int s, len;
	int t;
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

static int transmit_data(int socket)
{
	const char *led_command = "SYST:NET:LANLED:BLINK %s";
	const char *blink_on = "ON";
	const char *blink_off = "OFF";

	const char *set = "Set";
	char str[BUFFER_SIZE];

	// data that needs to be sent regardless of the
	// configuration condition

	if(strcmp(set, blink) == 0)
	{
		sprintf(str, led_command, blink_on);
	}
	else
	{		
		sprintf(str, led_command, blink_off);
	}

	send(socket, str, strlen(str), 0);
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

	printf("Content-Type: text/plain\r\n\r\n");
	printf("blink done");
}
