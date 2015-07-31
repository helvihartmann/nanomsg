#include <assert.h>
//#include <libc.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>
#include <stdbool.h>
#include <nanomsg/pair.h>
#include <nanomsg/tcp.h>
//#include "/home/hartmann/src/nanomsg-0.6-beta/tests/testutil.h"

#define RECEIVE "receive"
#define SEND "send"

int receive (const char *url)
{
  int sock0 = nn_socket (AF_SP, NN_PULL);
  //int sock0 = test_socket (AF_SP, NN_PAIR);
  assert (sock0 >= 0);
  nn_bind (sock0, url);
  bool end = false;
  while (!end){
    char *buf = NULL;
    int bytes = nn_recv (sock0, &buf, NN_MSG, 0);
    assert (bytes >= 0);
    printf ("RECEIVED \"%s\"\n", buf);
    if (strlen(buf) == 0){
        end = true;
    }
    nn_freemsg (buf);
  }
  return 0;
}

int send (const char *url, const char *msg)
{
  int sz_msg = strlen (msg) + 1; // '\0' too
  int sock1 = nn_socket (AF_SP, NN_PUSH);
  //int sock1 = test_socket (AF_SP, NN_PAIR);
  assert (sock1 >= 0);
  assert (nn_connect (sock1, url) >= 0);
  printf ("SENDING \"%s\"\n", msg);
  int bytes = nn_send (sock1, msg, sz_msg, 0);
  assert (bytes == sz_msg);
  int ret = nn_shutdown (sock1, 1);//int how = 0 in original but returns error
  if (ret != 0){
    printf("return code: %d\n", ret);
    printf ("nn_shutdwon failed with error code %d\n", nn_errno ());
  }
  return ret;
}

int main (const int argc, const char **argv)
{
    //printf ("bla\n");
    if (strncmp (RECEIVE, argv[1], strlen (RECEIVE)) == 0 && argc > 1){
      printf ("url %s \n",argv[2]);
      return receive (argv[2]);
    }
    else if (strncmp (SEND, argv[1], strlen (SEND)) == 0 && argc > 2){
      printf ("url %s\n",argv[2]);
      return send (argv[2], argv[3]);
    }
  else
    {
      fprintf (stderr, "Usage: pipeline %s|%s <URL> <ARG> ...'\n",
               RECEIVE, SEND);
      return 1;
    }
}
