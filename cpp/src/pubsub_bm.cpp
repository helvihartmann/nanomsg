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

using namespace std;
int checkbuf (const int *buf, int bytes);
void receive(int sock);
void send(int sock, size_t buffsize, size_t repeats);
int* createbuf(size_t bufsize);

void syncservice_client(const char *url){
    
    int sock1 = nn_socket (AF_SP, NN_REQ);
    if (sock1 < 0) std::cout << "nn_socket failed with error code " << nn_errno () << std::endl;
    int connect = nn_connect (sock1, "tcp://node0:6666");
    if (connect < 0) cout << "nn_connect failed with error code " << nn_strerror(nn_errno()) << " tcp://*:6666" << endl;
    
    int msg = 1024;
    int bytes = nn_send (sock1, &msg, sizeof(int), 0);
    
    int *buf = NULL;
    bytes = nn_recv (sock1, &buf, NN_MSG, 0);
    nn_freemsg (buf);

    int ret = nn_shutdown (sock1, 1);//int how = 0 in original but returns error
    if (ret != 0) cout << "nn_shutdwon failed with error code " << nn_strerror(nn_errno());
}

void syncservice_server(const char *url, int nmbrofsubscribers){
    int subscribers = 0;
    while (subscribers < nmbrofsubscribers) {
        int sock0 = nn_socket (AF_SP, NN_REP);
        if (sock0 < 0) cout << "nn_socket failed with error code " << nn_errno () << endl;
        int bind = nn_bind (sock0, "tcp://*:6666");
        if (bind < 0) cout << "nn_bind failed with error code " << nn_strerror(nn_errno()) << " tcp://node0:6666" << endl;

        int *buf = NULL;
        int bytes = nn_recv (sock0, &buf, NN_MSG, 0);
        nn_freemsg (buf);
        cout << "CLIENT: received request" << endl;
        
        int msg = 1024;
        bytes = nn_send (sock0, &msg, sizeof(int), 0);
        if(bytes < 0) cout << "SERVER: ERROR replying, " << nn_strerror(nn_errno()) << endl;
        cout << "CLIENT: replied" << endl;

        int ret = nn_shutdown (sock0, 1);//int how = 0 in original but returns error
        if (ret != 0) cout << "nn_shutdwon failed with error code " << nn_strerror(nn_errno());
        
        subscribers++;
    }
}


int pubserver (const char *url, Parameters *params){
    /*int bytes = 0;
    int sock = nn_socket (AF_SP, NN_PUB);
    assert (sock >= 0);
    assert (nn_bind (sock, url) >= 0);
    
    std::cout << "nn_bind succesfull " << url << std::endl;*/
    //syncservice_server(url, 2);
    syncservice_server(url, params->getnmbrsubs());
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