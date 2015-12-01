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
#include "socketmng.h"
#include <ctime>
#include <chrono>
#include <vector>
#include <limits>

#define RECEIVE "receive"
#define SEND "send"

using namespace std;
using namespace std::chrono;

int checkbuf (const int *buf, int bytes);
int* createbuf(size_t bufsize, int vote);
int close (int sock);
int open(const char *url, const char *socktype);

void send(const char *url, size_t bufsize, size_t repeatsfix, vector<size_t>messagsizes, Socketmng *sockets){

    int sock1 = sockets->open(url, push, connect);


    int *mymsg = createbuf(bufsize,1);
    size_t repeats = repeatsfix;
    int factor = 2;
    size_t bytes = 0;
    size_t index = 0;
    for (size_t sz_msg = messagsizes.front(); sz_msg < messagsizes.back(); sz_msg = sz_msg * factor){
        if (sz_msg >= 8192){
            repeats = (repeatsfix*1000)/sz_msg;
         }
        if (repeats <= 1) repeats = 2;
        
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        for (size_t i = 0; i < repeats; i++){
            index = (sz_msg*i)%bufsize;
            bytes = nn_send (sock1, (mymsg+index), sz_msg, 0);
        }
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        double time = time_span.count();
        
        cout << sz_msg*repeats << " " << repeats << " " << sz_msg << " " << time << " " << (sz_msg*repeats)/(time*1000000) << endl;
    }
    mymsg[0] = 0;
    nn_send (sock1, mymsg, 4, 0);
    free(mymsg);

    sockets->close(sock1);
}


void receive (const char *url, Socketmng *sockets){
    
    int sock0 = sockets->open(url, pull, bind);
    
    int bytes = 0;
    int checksum = 0;
    bool end = false;
    while (!end){

        int *buf = NULL;
        bytes = nn_recv (sock0, &buf, NN_MSG, 0);
        assert(bytes >= 0);
        if(buf[0] == 0) end = true;
        /*checksum = checkbuf(buf, bytes);
        if (checksum != 1){
            end = true;
            if (checksum != 0) printf ("ERROR occured, received wrong numbers, checksum = %d\n", checksum);
        }*/
        
        nn_freemsg (buf);
    }

    sockets->close(sock0);
}



int main (const int argc, char **argv)
{
    Parameters params(argc, argv);
    Socketmng sockets;

    const char *url = params.geturl();
    Type type = params.gettype();

    switch (type) {
        case server:{
            receive(url, &sockets);
            break;
        }
        case client:{
            send(url, params.getbuffersize(), params.getrepeats(), params.getmessagesizes(), &sockets);
        break;
        }
    default:
        fprintf (stderr, "Usage: pubsub %s|%s <URL> <ARG> ...\n",
                 SERVER, CLIENT);
        return 1;
    }
    return 0;
}
