#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <time.h>

// location for unix socket
#define SOCK_PATH "/var/tmp/web.socket"
#define COMMAND_PIPE "/var/tmp/cmd_pipe"
#define WEB_RESP_PIPE "/var/tmp/web_pipe"
#define SOCKET_CONNECTIONS 5
#define COMMAND_TERMINATOR ';'
#define QUERY_CHARACTER '?'
#define BUFFER_LENGTH 128
#define WEB_ID 1
#define FOREVER 1
#define TRUE 1
#define FALSE 0
#define TIMEOUT 2

#ifdef DEBUG_MODE
#undef DEBUG_MODE
#endif
// Function prototypes
static int is_query(char *string, int length);
static int read_scpi_reply(int response_fd, char *buffer, int length);

typedef struct 
{
	long caller;
	unsigned char str[BUFFER_LENGTH];
} SCPI_MSG;

static int is_query(char *string, int length)
{
	int i;

	// Scan the string for the query character
	for(i = 0; i < length; ++i)
	{
		if(string[i] == QUERY_CHARACTER)
		{
			return TRUE;
		}
	}

	// If we got here then we failed to find the query indicator
	return (FALSE);
}

static int read_scpi_reply(int response_fd, char *buffer, int length)
{
	char *current_index = buffer;

	int characters_read = 0;
	int char_read;
	time_t start, now;

	start = time(NULL);
	while(TRUE)
	{
		now = time(NULL);

		if(now - start >= TIMEOUT)
		{
			return 0;
		}

		// read out characters from the response pipe
		char_read = read(response_fd, current_index, length - characters_read);

		if(char_read > 0)
		{
			characters_read += char_read;
			current_index = buffer + characters_read;
		}

		if(char_read < 0)
		{
			if(errno == EAGAIN)
			{
				usleep(3000);
			}
			else
			{
				perror("Failed in response pipe read");
				usleep(3000);
			}
		}
		else
		{

			// wait for termination with carriage return, line feed
			if(characters_read >= 2 && buffer[characters_read - 2] == '\r' && buffer[characters_read - 1] == '\n')
			{
				return (characters_read - 2);
			}
		}
	}

	// Buffer overflow
	return (-1);
}

#ifdef DEBUG_MODE
static void debug_print(char *text)
{
	const char *debug_file_name = "/home/debug.txt";
	FILE *f;

	f = fopen(debug_file_name, "a");
	fprintf(f, text);
	fclose(f);
}
#endif

int main(void)
{
	int s, s2, t, len;
	struct sockaddr_un local, remote;
	SCPI_MSG scpi_msg;
	const char *failed_query_string = "failed_query";

scpi_msg.caller = WEB_ID;
// File descriptors for the command and response pipes
int cmd_fd, rsp_fd;

  // Get a socket descriptor
  if((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
  {
    perror("web Failed to form socket");
    exit(1);
  }

	// Copy socket location
	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);

	// delete the socket if it already exists
	unlink(local.sun_path);
	
	len = strlen(local.sun_path) + sizeof(local.sun_family);

  // Form the socket
  if(bind(s, (struct sockaddr *)&local, len) == -1)
  {
    perror("web Failed in bind operation");
    exit(1);
  }

  if(listen(s, SOCKET_CONNECTIONS) == -1)
  {
    perror("web Error during listen operation");
    exit(1);
  }

  // Form the response pipe
  unlink(WEB_RESP_PIPE);
  mkfifo(WEB_RESP_PIPE, S_IFIFO | 0666);
  rsp_fd = open(WEB_RESP_PIPE, O_RDONLY | O_NONBLOCK);

  if(rsp_fd < 0)
  {
    perror("web Failed open response fifo");
    exit(1);
  }

  // Let the parser set up
  usleep(5000);
  // Form the command pipe
  cmd_fd = open(COMMAND_PIPE, O_WRONLY);
  if(cmd_fd < 0)
  {
    perror("web Failed open command pipe");
    exit(1);
  }

  scpi_msg.caller = WEB_ID;
  scpi_msg.str[0] = COMMAND_TERMINATOR;

	write(cmd_fd, &scpi_msg, sizeof(long) + sizeof(char));

	// Let the parser set up
	usleep(10000);

	// Run the server forever
	while(FOREVER)
	{
		int done, n;
#ifdef DEBUG_MODE
char debug_string[256];
#endif

    t = sizeof(remote);
    // Make the socket connection
    if((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1)
    {
      perror("web Error during socket acceptance");
      exit(1);
    }

		done = FALSE;
		do
		{
			// Read the command to send out
			n = recv(s2, scpi_msg.str, BUFFER_LENGTH, 0);

      // Didn't get any characters or got an error
      if(n <= 0)
      {
        if(n < 0)
        {
          perror("web Error socket receive operation");
        }
        done = 1;
      }

      //  Successfully got characters so now ship them off
      // to the command pipe
      if(!done)
      {
        scpi_msg.caller = WEB_ID;
        // Append the command terminator
        scpi_msg.str[n++] = COMMAND_TERMINATOR;
#ifdef DEBUG_MODE
strncpy(debug_string, scpi_msg.str, n);
debug_string[n] = '\0';
debug_print(debug_string);
#endif
				// Send out the command to the parser
				write(cmd_fd, &scpi_msg, sizeof(long) + sizeof(char) * n);
				// If this is a query then we need to send some data back
				// Otherwise it was just a plain command
				if(is_query(scpi_msg.str, n))
				{
					// Now we need to read in the reply
					n = read_scpi_reply(rsp_fd, scpi_msg.str, BUFFER_LENGTH);
#ifdef DEBUG_MODE
strncpy(debug_string, scpi_msg.str, n);
debug_string[n] = '\0';
debug_print(debug_string);
#endif
          // Send out an error indication if we timed out
          if(n == 0)
          {
            send(s2, failed_query_string, strlen(failed_query_string), 0);
            done = TRUE;
          }
          else if(send(s2, scpi_msg.str, n, 0) < 0)
          {
            // Failure on sending the reply back to the socket
            // connection
            perror("web Failed on sending socket reply");
            done = TRUE;
          }
        }
      }
    } while(!done);
  }

	// Not expecting to ever get here
	return (1);
}
