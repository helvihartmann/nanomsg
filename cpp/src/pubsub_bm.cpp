#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include "parameters.h"
#include <sys/time.h>


int checkbuf (const int *buf, int bytes);
void receive(int sock);
void send(int sock, size_t buffsize, size_t repeats);

int pubserver (const char *url, Parameters *params){
    int bytes = 0;
    int sock = nn_socket (AF_SP, NN_PUB);
    assert (sock >= 0);
    assert (nn_bind (sock, url) >= 0);
    
    send(sock, params->getbuffersize(), params->getrepeats());
}

int subclient (const char *url, const char *name){
    int sock = nn_socket (AF_SP, NN_SUB);
    assert (sock >= 0);
    
    assert (nn_setsockopt (sock, NN_SUB, NN_SUB_SUBSCRIBE, "", 0) >= 0);
    assert (nn_connect (sock, url) >= 0);
    
    receive(sock);
}


int main (const int argc, char **argv){
    
    Parameters params(argc, argv);
    Type type = params.gettype();
    const char *url = params.geturl();
    switch (type) {
        case server:
            pubserver(url, &params);
            break;
        case client:
            subclient (url, params.getname());
            break;
        default:
            fprintf (stderr, "Usage: pubsub %s|%s <URL> <ARG> ...\n",
                     SERVER, CLIENT);
            return 1;
            break;
    }
}