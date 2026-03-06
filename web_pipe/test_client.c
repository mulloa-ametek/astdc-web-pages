#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/var/tmp/web.socket"

int main(void)
{
	int s, len;
	int t;
	struct sockaddr_un remote;
	char str[100];
char *idn = "*IDN?";	

	if((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if(connect(s, (struct sockaddr *)&remote, len) == -1)
	{
		perror("connect failure");
		exit(1);
	}

	while(1)
	{
		// Give a pause
		usleep(10000);

		//send(s, str, strlen(str), 0);
		send(s, idn, strlen(idn), 0);

		if((t = recv(s, str, 100, 0)) > 0)
		{
			str[t] = '\0';
			printf("echo> %s", str);
		}
	}

	close(s);

	return 0;
}
