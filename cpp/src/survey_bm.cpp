#include <assert.h>
//#include <libc.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/survey.h>
#include "parameters.h"


#define SERVER "server"
#define CLIENT "client"
#define DATE   "DATE"

int checkbuf (const int *buf, int bytes);
char *date();
void receive(int sock);
void send(int sock, size_t buffsize);

int surveyor(const char *url){
    
    //--------------------------------------------------------------------------
    int sock = nn_socket (AF_SP, NN_SURVEYOR);
    assert (sock >= 0);
    assert (nn_bind (sock, url) >= 0);
    sleep(3); // wait for connections
    std::cout << "SERVER: SENDING SURVEY REQUEST" << std::endl;
    int *surveyreq;
    surveyreq[0] = 5;
    int bytes = nn_send (sock, surveyreq, 4, 0);
    assert (bytes == 4);
    //--------------------------------------------------------------------------
    std::cout << "SERVER: checkpoint 1" << std::endl;

    receive(sock);
    std::cout << "SERVER: checkpoint 6" << std::endl;
}

int respondent (const char *url, const char *name, Parameters *params){
    
    //--------------------------------------------------------------------------
    int sock = nn_socket (AF_SP, NN_RESPONDENT);
    assert (sock >= 0);
    assert (nn_connect (sock, url) >= 0);
    
    int *buf = NULL;
    int bytes = nn_recv (sock, &buf, NN_MSG, 0);
    assert (bytes == 4);
    
    //std::cout << "CLIENT (" << name << ") RECEIVED " << buf[0] << " SURVEY REQUEST" << std::endl;
    nn_freemsg (buf);

    //--------------------------------------------------------------------------
    
    send(sock, params->getbuffersize());
}

int main (const int argc, char **argv)
{
    Parameters params(argc, argv);
    Type type = params.gettype();
    const char *url = params.geturl();
    switch (type) {
        case server:
            surveyor(url);
            break;
        case client:
            respondent (url, params.getname(), &params);
            break;
        default:
            fprintf (stderr, "Usage: pubsub %s|%s <URL> <ARG> ...\n",
                     SERVER, CLIENT);
            return 1;
            break;
    }
}