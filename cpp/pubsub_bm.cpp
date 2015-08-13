#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include "parameters.h"
#include <sys/time.h>


#define SERVER "server"
#define CLIENT "client"

char *date ()
{
    time_t raw = time (&raw);
    struct tm *info = localtime (&raw);
    char *text = asctime (info);
    text[strlen(text)-1] = '\0'; // remove '\n'
    return text;
}

int server (const char *url, Parameters *params)
{
    int bytes = 0;
    int sock = nn_socket (AF_SP, NN_PUB);
    assert (sock >= 0);
    assert (nn_bind (sock, url) >= 0);
    
    struct timeval start, end;
    int factor = 2;
    size_t endsz_msg = 16;
    size_t startsz_msg = 4;
    size_t repeats = 10;
    /*size_t repeats = params->getrepeats();
    size_t bufsize = params->getbuffersize();
    int *mymsg;
    mymsg = (int *) malloc(sizeof(*mymsg) * bufsize);
    
    for (int i = 0; i < bufsize; i++){
        mymsg[i] = 1;
    }
    printf ("Created buff of size %lu Bytes; %lu ints of size %lu Bytes \"\n", bufsize, bufsize/sizeof(*mymsg), sizeof(*mymsg));*/
    

    char *mymsg = date();
    int sz_d = strlen(mymsg) + 1; // '\0' too

    for (size_t sz_msg = startsz_msg; sz_msg < endsz_msg; sz_msg = sz_msg * factor){
        
        //gettimeofday(&start, NULL);
        
        for(int i = 0; i < repeats; i++){
            
            //printf ("SERVER: PUBLISHING DATE %s\n", msg);
            bytes = nn_send (sock, mymsg, sz_d, 0);
            assert (bytes == sz_d);
            sleep(1);
        }
        //gettimeofday(&end, NULL);
        
        //float time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
        //printf("%lu %lu %lu %.3f %lf \n",repeats*sz_msg, repeats, sz_msg, time/1000000, sz_msg/time);
        /*repeats = repeats/2;
        if (sz_msg >= 1048576){
            repeats = repeats/sz_msg;
        }
        if (repeats == 0) repeats = 2;*/
    }
    
    
    //free(mymsg);
    
    nn_send (sock, "", 1, 0);
    return nn_shutdown(sock, 0);
}

int client (const char *url, const char *name)
{
    int sock = nn_socket (AF_SP, NN_SUB);
    assert (sock >= 0);
    // TODO learn more about publishing/subscribe keys
    assert (nn_setsockopt (sock, NN_SUB, NN_SUB_SUBSCRIBE, "", 0) >= 0);
    assert (nn_connect (sock, url) >= 0);
    bool end = false;
    while (!end)
    {
        char *buf = NULL;
        int bytes = nn_recv (sock, &buf, NN_MSG, 0);
        assert (bytes >= 0);
        printf ("CLIENT (%s): RECEIVED %s\n", name, buf);
        if (strlen(buf) == 0) end = true;
        nn_freemsg (buf);
    }
    return nn_shutdown (sock, 0);
}

int main (const int argc, char **argv){
    
    Parameters params(argc, argv);

    if (strncmp (SERVER, argv[1], strlen (SERVER)) == 0 && argc >= 2) server(argv[2], &params);

    else if (strncmp (CLIENT, argv[1], strlen (CLIENT)) == 0 && argc >= 3)
        return client (argv[2], argv[3]);
    else
    {
        fprintf (stderr, "Usage: pubsub %s|%s <URL> <ARG> ...\n",
                 SERVER, CLIENT);
        return 1;
    }
}