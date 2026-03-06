// main.c - rfpsock

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>

#define debug 0
#define debufpf 0
#if debug | debufpf
FILE *logfp;
#endif

#define _MAIN_C_
//#include "main.h"

//void *cmdThread(void *arg);
void cmdThread(void);
void *rspThread(void *arg);

#define CMD_PIPE_NAME   "var/tmp/cmd_pipe"
#define RSP_PIPE_NAME   "/var/tmp/rsp_pipe5"
static int mypid = 5;

static int bRun = 1;
char rpipename[64];

int sockfd;

// *****************************************************************************
int atoh(char* buf)   // convert ascii NNNNN to hex number, #h/0x pre skipped
{
unsigned int val = 0;
unsigned char c;

  while (*buf)      // while chars to process
  {
    if (*buf >='0' && *buf<='9')
    {
      c = '0';
    }
    else
    {
      *buf &= ~' ';              // convert to upper
      if (*buf>='A' && *buf<='F')
      {
        c = '7';       // -'A' + 10
      }
      else
        return val;
    }
    val = (val<<4)|(*buf++ - c);  // store digit
  }

  return val;
}

// *****************************************************************************
int main(int argc, char *argv[])
{
pthread_t threads[2];
int fd = open(CMD_PIPE_NAME, O_WRONLY);

  if (fd<0)     // If Controller software is not yet up and running, just exit
    _exit(0);

  close(fd);

#if debug
logfp = fopen("/dev/ttyS3", "w");
#endif
//  mypid = getpid();

//printf("fp's %p, %p\n", stdin, stdout);

  pthread_create(&threads[1], NULL, rspThread, NULL);
  cmdThread();
  pthread_join(threads[1], NULL/*(void**)&status*/);

#if debug
  fclose(logfp);
#endif
  return 0;
}

// *****************************************************************************

typedef struct {
  long caller;
  unsigned char buf[1024];
} sockmsgtype;

// *****************************************************************************
// cmdThread() receives msgs from remote host and passes them along
// to the parser

//void *cmdThread(void *arg)
void cmdThread(void)
{
int fd, cnt=0, val;
sockmsgtype sockmsg;
#if debug
char ebuf[128];

  sprintf(ebuf, "cmdThread= %d\n", mypid);
  fputs(ebuf, logfp);
#endif
  fd = open(CMD_PIPE_NAME, O_WRONLY);
  if (fd > 0)
  { // Send one inital empty message to announce our presence.  This allows the
  // RFPparser to create all of the connection support i.e. errorqueue, etc...
  // and maintain a maximum number of connections.  When the max is reached,
  // the least recently used connection, will be severed.
  // See RfpParser's file Remote.c for details. 

    sockmsg.caller = mypid;
    sockmsg.buf[0] = ';';
    write(fd, &sockmsg, 1 + sizeof(long));
    usleep(10000);

    while (bRun)
    {
      errno = 0;
      val = fgetc(stdin);
      if(val < 0)           // if EOF
      {
        bRun = 0;
        cnt = sprintf(sockmsg.buf, "tremote %d;", mypid);
        write(fd, &sockmsg, cnt + sizeof(long));      // send to parser
        for(;;)
          sleep(4);     // allow the parser to SIGTERM us, should never return
      }

      if(val == '\r' || val == '\n')
      {
send:   if(!cnt)                  // strip leading terminators
          continue;

        sockmsg.buf[cnt++] = ';'; // add terminator
        if(cnt> 1)                // allow inpThread() to filter runts
        {
          write(fd, &sockmsg, cnt + sizeof(long));      // send to parser
#if debufpf
          sockmsg.buf[cnt++] = '\x00';
          fputs(sockmsg.buf, logfp);
          fflush(logfp);
#endif
        }
        cnt = 0;                    // read a new record
        continue;
      }
      if(val == ';')
        goto send;

      sockmsg.buf[cnt++] = (unsigned char)val;

      if(cnt>1020)      // very big command, just send...
        goto send;
    }

    close(fd);
  }
//  pthread_exit(NULL); 
}

// *****************************************************************************
// rspThread() receives a response from the parser and passes it back
// to the remote host. The parser does an initial open then just writes
#include <sys/socket.h>

void *rspThread(void *arg)
{
int fd, cnt;
unsigned char msgbuf[1024];
#if debug
char ebuf[128];

  sprintf(ebuf, "rspThread= %d\n", getpid());
  fputs(ebuf, logfp);
#endif

  // response pipes are unique for each interface
	mkfifo(RSP_PIPE_NAME, S_IFIFO | 0666);

  fd = open(RSP_PIPE_NAME, O_RDONLY);
  if (fd > 0)
  {
    usleep(5000);

    while (bRun)
    {
      cnt = read(fd, msgbuf, 1022);   // read from rfpparser
      if (cnt < 1)                    // read I/O error, bail out
      {
        bRun = 0;
        break;
      }

      errno = 0;
		msgbuf[cnt] = '\x00';         // terminate the string
      cnt = fputs(msgbuf, stdout);  // send to remote
      fflush(stdout);

      if(cnt<1)                       // write I/O error, bail out
        bRun = 0;
#if debug
      if(cnt < 0)
      {
        strerror_r(errno, ebuf, 128);
        fputs(ebuf, logfp);
        fflush(logfp);
      }
      fputs(msgbuf, logfp);
      fflush(logfp);
#endif
    }  // end     while (bRun)

    close(fd);
    unlink(rpipename);
  }
  pthread_exit(NULL);
}

