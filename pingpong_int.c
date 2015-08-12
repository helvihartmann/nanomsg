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
#include <time.h>
//#include "/home/hartmann/src/nanomsg-0.6-beta/tests/testutil.h"

#define RECEIVE "receive"
#define SEND "send"

int checkbuf (const int *buf, int bytes){
    int j = 0;
    int sum = 0;
    for ( int i = 0; i < bytes/4; i++){
        j = buf[i];
        //printf ("%d", j);
        sum = sum + j;
    }
    //printf ("\n");
    
    return sum/(bytes/sizeof(int));
}

int receive (const char *url)
{
    int sock0 = nn_socket (AF_SP, NN_PULL);
    //int sock0 = test_socket (AF_SP, NN_PAIR);
    assert (sock0 >= 0);
    nn_bind (sock0, url);
    bool end = false;
    while (!end){

        int *buf = NULL;
        int bytes = nn_recv (sock0, &buf, NN_MSG, 0);

        int checksum = checkbuf(buf, bytes);
        if (checksum != 1){
            end = true;
            if (checksum != 0) printf ("ERROR occured, received wrong numbers, checksum = %d\n", checksum);
        }
        
        nn_freemsg (buf);
    }
    return 0;
}

int send (const char *url, const int *msg, size_t sz_msg)
{
    int sock1 = nn_socket (AF_SP, NN_PUSH);
    //int sock1 = test_socket (AF_SP, NN_PAIR);
    assert (sock1 >= 0);
    assert (nn_connect (sock1, url) >= 0);


    //printf ("SENDING \"%d (%lu bytes)\"\n", *msg, sz_msg);

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
    time_t now, end;
    struct tm * timeinfo;
    double seconds;
    
    if (strncmp (RECEIVE, argv[1], strlen (RECEIVE)) == 0 && argc > 1){
        //printf ("url %s \n",argv[2]);
        return receive(argv[2]);
    }
    else if (strncmp (SEND, argv[1], strlen (SEND)) == 0 && argc > 2){
        //printf("url %s\n",argv[2]);
        
        int bufsize = 536870912;
        int *mymsg;
        mymsg = (int *) malloc(sizeof(*mymsg) * bufsize);
        
        for (int i = 0; i < bufsize; i++){
            mymsg[i] = 1;
        }
        printf ("Created buff of size %lu Bytes; %lu ints of size %lu Bytes \"\n", bufsize, bufsize/sizeof(*mymsg), sizeof(*mymsg));
        
        for (size_t sz_msg = 4; sz_msg < 1024*1024; sz_msg = sz_msg * 2){
            
            time(&now);  /* get current time; same as: now = time(NULL)  */
            timeinfo = localtime(&now);
            printf ("%d seconds \n", timeinfo->tm_sec);
            
            for (size_t repeats = 0; repeats < 100; repeats++){
                send(argv[2], mymsg, sz_msg);
            }
            end = time(0);
            timeinfo = localtime(&end);
            
            printf ("%d.\n", timeinfo->tm_sec);

            
            time(&end);
            seconds = difftime(now,end);
            
            printf ("SENDING \"%lu bytes took %f seconds\"\n", sz_msg, seconds);
        }
        
        mymsg[0] = 0;
        send(argv[2], mymsg, 4);
        free(mymsg);
        
        return 0;
    }
  else
    {
      fprintf (stderr, "Usage: pipeline %s|%s <URL> <ARG> ...'\n",
               RECEIVE, SEND);
      return 1;
    }
}
