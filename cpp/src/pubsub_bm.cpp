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
void send(int sock, size_t buffsize);

int pubserver (const char *url, Parameters *params)
{
    int bytes = 0;
    int sock = nn_socket (AF_SP, NN_PUB);
    assert (sock >= 0);
    assert (nn_bind (sock, url) >= 0);
    
    send(sock, params->getbuffersize());
    /*struct timeval start, end;
    int factor = 2;
    size_t endsz_msg = 2<<30;
    size_t startsz_msg = 4;
    size_t repeats = 2;
    //size_t repeats = params->getrepeats();
    size_t bufsize = params->getbuffersize();
    int *mymsg;
    mymsg = (int *) malloc(sizeof(*mymsg) * bufsize);
    
    for (int i = 0; i < bufsize; i++){
        mymsg[i] = 1;
    }
    std::cout << "Created buff of size " << bufsize << " Bytes" << std::endl;
    
    for (size_t sz_msg = startsz_msg; sz_msg < endsz_msg; sz_msg = sz_msg * factor){
        
        gettimeofday(&start, NULL);
        
        for(int i = 0; i < repeats; i++){
            bytes = nn_send (sock, mymsg, sz_msg, 0);
            //std::cout << bytes << std::endl;
            //assert (bytes == sz_msg);
            sleep(1);
        }
        gettimeofday(&end, NULL);
        
        float time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
        std::cout << repeats*sz_msg << " " << repeats << " " << sz_msg << " " << time/1000000 << " " << sz_msg/time << std::endl;
    }
    
    mymsg[0] = 0;
    nn_send (sock, mymsg, 4, 0);
    return nn_shutdown(sock, 0);
    free(mymsg);*/
}

int subclient (const char *url, const char *name)
{
    int sock = nn_socket (AF_SP, NN_SUB);
    assert (sock >= 0);
    // TODO learn more about publishing/subscribe keys
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