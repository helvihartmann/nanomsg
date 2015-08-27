#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pair.h>
#include <nanomsg/tcp.h>
#include "parameters.h"
#include <sys/time.h>
#include <errno.h>

int checkbuf (const int *buf, int bytes);
void receive(int sock);
void send(int sock, size_t buffsize, size_t repeats);


void syncservice_client(const char *url){
    
    int bytes = 0;
    int sock = nn_socket (AF_SP, NN_PULL);
    
    if (sock < 0){
        std::cout << "return code: " << sock << std::endl;
        std::cout << "nn_socket failed with error code " << nn_errno () << std::endl;
    }
    
    int bind = nn_bind (sock, "tcp://*:6666");
    
    if (bind < 0) std::cout << "nn_bind failed with error code " << nn_strerror(nn_errno()) << " tcp://node0:6666" << std::endl;
    
    int *buf = NULL;
    bytes = nn_recv (sock, &buf, NN_MSG, 1);
    nn_freemsg (buf);
    
    //int msg = 1024;
    //bytes = nn_send (sock, &msg, sizeof(int), 1);
    
    nn_shutdown (sock, 1);
}


void syncservice_server(const char *url, int nmbrofsubscribers){
    
    int bytes = 0;
    int sock = nn_socket (AF_SP, NN_PUSH);
    
    if (sock < 0){
        std::cout << "return code: " << sock << std::endl;
        std::cout << "nn_socket failed with error code " << nn_errno () << std::endl;
    }
    
    int connect = nn_connect (sock, "tcp://node0:6666");
    
    if (connect < 0) std::cout << "nn_connect failed with error code " << nn_strerror(nn_errno()) << " tcp://*:6666" << std::endl;

    
    int subscribers = 1;
    while (subscribers < nmbrofsubscribers) {
        
        int msg = 1024;
        bytes = nn_send (sock, &msg, sizeof(int), 1);
        
        /*int *buf = NULL;
        bytes = nn_recv (sock, &buf, NN_MSG, 1);
        nn_freemsg (buf);*/
        subscribers++;
    }
    
    nn_shutdown (sock, 1);
}



int pubserver (const char *url, Parameters *params){
    /*int bytes = 0;
    int sock = nn_socket (AF_SP, NN_PUB);
    assert (sock >= 0);
    assert (nn_bind (sock, url) >= 0);
    
    std::cout << "nn_bind succesfull " << url << std::endl;*/
    syncservice_server(url, 3);
    //syncservice_server(params->getnmbrofsubscribers());
    //send(sock, params->getbuffersize(), params->getrepeats());
}

int subclient (const char *url, const char *name){
    /*int sock = nn_socket (AF_SP, NN_SUB);
    assert (sock >= 0);
    
    
    assert (nn_setsockopt (sock, NN_SUB, NN_SUB_SUBSCRIBE, "", 0) >= 0);
    assert (nn_connect (sock, url) >= 0);
    
    std::cout << "nn_connect succesfull " << name << " " << url << std::endl;*/
    syncservice_client(url);
    //receive(sock);
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