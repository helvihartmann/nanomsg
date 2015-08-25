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
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include "parameters.h"

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

int receive (const char *url){
    int sock0 = nn_socket (AF_SP, NN_PULL);
    //int sock0 = nn_socket (AF_SP, NN_PAIR);
    assert (sock0 >= 0);
    nn_bind (sock0, url);
    
    int bytes = 0;
    int checksum = 0;
    bool end = false;
    while (!end){

        int *buf = NULL;
        bytes = nn_recv (sock0, &buf, NN_MSG, 0);

        checksum = checkbuf(buf, bytes);
        if (checksum != 1){
            end = true;
            if (checksum != 0) printf ("ERROR occured, received wrong numbers, checksum = %d\n", checksum);
        }
        
        nn_freemsg (buf);
    }
    return 0;
}

int send (const char *url, const int *msg, size_t sz_msg, size_t repeats)
{
    int sock1 = nn_socket (AF_SP, NN_PUSH);
    //int sock1 = nn_socket (AF_SP, NN_PAIR);
    if (sock1 < 0){
        printf("return code: %d\n", sock1);
        printf ("nn_socket failed with error code %d\n", nn_errno ());
        return 0;
    }
    assert (nn_connect (sock1, url) >= 0);

    int bytes = 0;
    for (size_t i = 0; i < repeats; i++){
        bytes = nn_send (sock1, msg, sz_msg, 0);
    }
    assert (bytes == sz_msg);
    int ret = nn_shutdown (sock1, 1);//int how = 0 in original but returns error
    if (ret != 0){
        printf("return code: %d\n", ret);
        printf ("nn_shutdwon failed with error code %d\n", nn_errno ());
    }
    return ret;
}

int main (const int argc, char **argv)
{
    struct timeval start, end;
    Parameters params(argc, argv);
    const char *url = params.geturl();
    Type type = params.gettype();

    switch (type) {
        case server:{
            receive(url);
            break;
        }
        case client:{
            size_t bufsize = params.getbuffersize();
            int *mymsg;
            mymsg = (int *) malloc(sizeof(*mymsg) * bufsize);
            
            for (int i = 0; i < bufsize; i++){
                mymsg[i] = 1;
            }
            printf ("Created buff of size %lu Bytes; %lu ints of size %lu Bytes \"\n", bufsize, bufsize/sizeof(*mymsg), sizeof(*mymsg));
            
            //size_t repeats = 2000000;
            size_t repeatsfix = params.getrepeats();
            size_t repeats = 0;
            int factor = 2;
            for (size_t sz_msg = 4; sz_msg < 1024*1024*1024; sz_msg = sz_msg * factor){
                repeats = (repeatsfix*100)/sz_msg;
                /*if (sz_msg >= 1048576){
                    repeats = (repeatsfix*10000)/sz_msg;
                }*/
                if (repeats <= 1) repeats = 2;
                
                gettimeofday(&start, NULL);
                send(url, mymsg, sz_msg, repeats);
                gettimeofday(&end, NULL);
                
                float time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
                //printf("%lu %lu %lu %.3f %lf \n",repeats*sz_msg, repeats, sz_msg, time/1000000, sz_msg/time);
                std::cout << repeats*sz_msg << " " << repeats << " " << sz_msg << " " << time/1000000 << " " << (repeats*sz_msg)/time << std::endl;
            }
            
            mymsg[0] = 0;
            send(url, mymsg, 4, 1);
            free(mymsg);
            break;
        }
        default:
            fprintf (stderr, "Usage: pubsub %s|%s <URL> <ARG> ...\n",
                     SERVER, CLIENT);
            return 1;
            break;
    }
    
    /*if (strncmp (RECEIVE, argv[1], strlen (RECEIVE)) == 0 && argc > 1){
        //printf ("url %s \n",argv[2]);
        return receive(argv[2]);
    }
    else if (strncmp (SEND, argv[1], strlen (SEND)) == 0 && argc > 2){
        //printf("url %s\n",argv[2]);
        
        //size_t bufsize = 2147483648;
        size_t bufsize = params.getbuffersize();
        int *mymsg;
        mymsg = (int *) malloc(sizeof(*mymsg) * bufsize);
        
        for (int i = 0; i < bufsize; i++){
            mymsg[i] = 1;
        }
        printf ("Created buff of size %lu Bytes; %lu ints of size %lu Bytes \"\n", bufsize, bufsize/sizeof(*mymsg), sizeof(*mymsg));
        
        //size_t repeats = 2000000;
        size_t repeats = params.getrepeats();

        int factor = 2;
        for (size_t sz_msg = 4; sz_msg < 1024*1024*1024; sz_msg = sz_msg * factor){
    
            gettimeofday(&start, NULL);
            send(argv[2], mymsg, sz_msg, repeats);
            gettimeofday(&end, NULL);
            
            float time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
            printf("%lu %lu %lu %.3f %lf \n",repeats*sz_msg, repeats, sz_msg, time/1000000, sz_msg/time);
            repeats = repeats/2;
            if (sz_msg >= 1048576){
                repeats = repeats/sz_msg;
            }
            if (repeats == 0) repeats = 2;
        }
        
        mymsg[0] = 0;
        send(argv[2], mymsg, 4, 1);
        free(mymsg);
        
        return 0;
    }
  else
    {
      fprintf (stderr, "Usage: pipeline %s|%s <URL> <ARG> ...'\n",
               RECEIVE, SEND);
      return 1;
    }*/
}
