//////////////////////////////////////////////////////////////////////////
// Title:				module_list.c
// Author:				Adam Mocarski
// Description:		This module is for implementing the CGI behind
//							printing the list of modules on the "Modules" page
//							of the website
//
// Copyright (C) AMETEK Programmable Power, 2014
//////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define ERROR 0
#define SUCCESS 1
#define TRUE 1
#define FALSE 0

// location of the socket to execution pipe
#define SOCK_PATH "/var/tmp/web.socket"
// 100 character buffer to internal socket
#define BUFFER_SIZE 100
#define IDN_SIZE 10

// Function prototypes
static int form_socket(void);
static int transmit_data(int socket);
int handle_first_slot_and_pop(char **slot_pointer, int socket);

////////////////////////////////////////////////////////////////////
// This function will open up a socket to the web pipe.
// This will allow it access to the SCPI parser
//
// The function returns the handle to the socket that was opened
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
// This function will get a list of all the modules.  And then it
// will iterate through the list, performing a *IDN? on each of 
// the modules
//
// Returns ERROR on a failure or SUCCESS on success
////////////////////////////////////////////////////////////////////
static int transmit_data(int socket)
{
	char socket_list[BUFFER_SIZE];
	char *p;
	int bytes_back;
	// This is the SCPI command to get a list of modules
	const char *get_module_list = "EIB:CONF:LADD?";
	int error_code;

	// Ask to get a list of all the module slots in the
	// system.  This will come back as a string like
	// "4,5,7"  First number is the controller
	send(socket, get_module_list, strlen(get_module_list), 0);

	bytes_back = recv(socket, socket_list, BUFFER_SIZE, 0);

	if(bytes_back <= 0)
	{
		return ERROR;
	}

	// Null terminate the string
	socket_list[bytes_back] = '\0';
	p = socket_list;

	// p will iterate across the socket_list
	// and perform a *IDN? on each entry
	for(p = socket_list; *p != '\0';)
	{
		error_code = handle_first_slot_and_pop(&p, socket);

		if(error_code = ERROR)
		{
			return ERROR;
		}
	}

	return SUCCESS;
}

////////////////////////////////////////////////////////////////////
// slot_pointer holds the list of modules
// this function will read it in, parse it, and then do a *IDN?
// on the value
////////////////////////////////////////////////////////////////////
int handle_first_slot_and_pop(char **slot_pointer, int socket)
{
	int slot;
	// WIll append the module number to the *IDN to get the *IDN?
	// string from a particular module
	const char *idn_string = "*idn%d?";
	char str[BUFFER_SIZE];
	int bytes_back;

	slot = 0;

	// convert the string to interger
	while(**slot_pointer <= '9' && **slot_pointer >= '0')
	{
		slot = 10 * slot + (**slot_pointer - '0');
		++(*slot_pointer);
	}

	// Perform some error checking
	switch(**slot_pointer)
	{
	case '\0':
		break;
	case ',':
		// jump past the ','
		++(*slot_pointer);
		break;
	default:
		return ERROR;
		break;
	}
	
	// Insert the module number to the *IDN? string
	sprintf(str, idn_string, slot);
	// Send the SCPI command to the parser
	send(socket, str, strlen(str), 0);

	// prepend the slot number to the *IDN? result
	// A later script will get things into the proper 
	// form
	printf("%d,", slot);
	bytes_back = recv(socket, str, BUFFER_SIZE, 0);
	if(bytes_back <= 0 || bytes_back > BUFFER_SIZE - 2)
	{
		return ERROR;
	}
	
	// Make a string out of this so puts can handle it
	str[bytes_back] = '\0';
	puts(str);

	return SUCCESS;
}

int main(void)
{
	int s; // The socket handle
	
	// Open connection to the parser vis UNIX socket
	s = form_socket();
	// Send out the programming commands to the scpi parser
	transmit_data(s);

	// Close the socket connection
	close(s);

	return (SUCCESS);
}
