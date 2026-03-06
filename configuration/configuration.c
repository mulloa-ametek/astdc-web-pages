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
#define POST_ENTRIES 7
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
static int dhcp;
static int auto_ip;
static char *host_name;
static char *description;
static char *ip_address;
static char *subnet_mask;
static char *gateway;
static char *dns_server;

static int get_cgi_data(void)
{
	//  CGI variable names
	const char *host_name_name = "host_name";
	const char *description_name = "description";
	const char *address_configuration_name = "address_configuration";
	const char *ip_address_name = "ip_address";
	const char *subnet_mask_name = "subnet_mask";
	const char *gateway_name = "gateway";
	const char *dns_server_name = "dns_server";
	const char *dhcp_mode_name = "dhcp";
	const char *auto_ip_checkbox_name = "auto_ip_checkbox";
	char *dhcp_cgi;
	int entries_read;

	char *print_buf[256];
	FILE *fs;

	fs = fopen("/var/weblog", "a");

	entries_read = scan_input(cgi_data, POST_ENTRIES);

	if(entries_read == 0)
	{
		// failed to read any values.  Something went wrong
		return ERROR;
	}

	// Parameters that are common to both DHCP and Static IP configurations
	host_name = get_record(cgi_data, host_name_name, POST_ENTRIES);
	description = get_record(cgi_data, description_name, POST_ENTRIES);

	if(host_name == NULL || description == NULL)
	{
		return -1;
	}
	
	// Are we in a DHCP condition?
	dhcp_cgi = get_record(cgi_data, address_configuration_name, POST_ENTRIES);
	fprintf(fs,"dhcp_cgi : %s, dhcp_mode_name : %s\n", dhcp_cgi, dhcp_mode_name);
	fclose(fs);
	if(dhcp_cgi == NULL)
	{
		// There was an error
		return -1;
	}
	else if(strcmp(dhcp_cgi, dhcp_mode_name) == 0)
	{
		dhcp = TRUE;
	}
	else
	{
		dhcp = FALSE;
	}

	if(dhcp)
	{
		// Handle all of the DHCP parameters

		// auto_ip
		if(get_record(cgi_data, auto_ip_checkbox_name, POST_ENTRIES) != NULL)
		{
			auto_ip = TRUE;
		}
		else
		{
			auto_ip = FALSE;
		}
	}
	else
	{
		// Static IP parameters
		ip_address = get_record(cgi_data, ip_address_name, POST_ENTRIES);
		subnet_mask = get_record(cgi_data, subnet_mask_name, POST_ENTRIES);
		gateway = get_record(cgi_data, gateway_name, POST_ENTRIES);
		dns_server = get_record(cgi_data, dns_server_name, POST_ENTRIES);
	}
	return 0;
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
	const char *address_command = "SYST:NET:IP \"%s\"";
	const char *autoip_command = "SYST:NET:AUTOIP %i";
	const char *description_command = "SYST:NET:DESC \"%s\"";
	const char *dhcp_command = "SYST:NET:DHCPMODE %i";
	const char *dns_command = "SYST:NET:DNS \"%s\"";
	const char *gateway_command = "SYST:NET:GATE \"%s\"";
	const char *hostname_command = "SYST:NET:HOST \"%s\"";
	const char *subnet_command = "SYST:NET:MASK \"%s\"";
	const char *apply_command = "SYST:NET:APPLY";

	char str[BUFFER_SIZE];

	// data that needs to be sent regardless of the
	// configuration condition

	// hostname
	sprintf(str, hostname_command, host_name);
	send(socket, str, strlen(str), 0);
	usleep(10000);

	// description
	sprintf(str, description_command, description);
	send(socket, str, strlen(str), 0);
	usleep(10000);

	// Case of a DHCP connection
	sprintf(str, dhcp_command, dhcp);
	send(socket, str, strlen(str), 0);
	usleep(10000);

	if(dhcp)
	{
		// For DHCP, will we allow autoip?
		sprintf(str, autoip_command, auto_ip);
		send(socket, str, strlen(str), 0);
		usleep(10000);
	}
	else
	{
		// This is a fixed IP address
		sprintf(str, address_command, ip_address);
		send(socket, str, strlen(str), 0);
		usleep(10000);

		// This is a fixed subnet address
		sprintf(str, subnet_command, subnet_mask);
		send(socket, str, strlen(str), 0);
		usleep(10000);

		// This is a fixed IP address
		sprintf(str, dns_command, dns_server);
		send(socket, str, strlen(str), 0);
		usleep(10000);

		// This is a fixed IP address
		sprintf(str, gateway_command, gateway);
		send(socket, str, strlen(str), 0);
		usleep(10000);
	}

	sprintf(str, apply_command);
	send(socket, str, strlen(str), 0);
	usleep(10000);
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

	// Send out the new web page
	//execlp("./lxi_identification", "lxi_identification", "/programming_done.xsl");
	//execlp("./lxi_configuration", "lxi_configuration", "/configuration.xsl", (char *)0);
	execlp("./lxi_configuration", "lxi_configuration", "/programming_done.xsl");
}
