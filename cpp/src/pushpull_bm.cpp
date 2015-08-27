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
#include <ctime>
#include <chrono>
#include <vector>

#define RECEIVE "receive"
#define SEND "send"

using namespace std;
using namespace std::chrono;


int checkbuf (const int *buf, int bytes);
int* createbuf(size_t bufsize);

int send (const char *url, const int *msg, size_t sz_msg, size_t repeats)
{
    int sock1 = nn_socket (AF_SP, NN_PUSH);
    if (sock1 < 0)cout << "nn_socket failed with error code " << nn_strerror(nn_errno());
    assert (nn_connect (sock1, url) >= 0);
    
    int bytes = 0;
    for (size_t i = 0; i < repeats; i++){
        bytes = nn_send (sock1, msg, sz_msg, 0);
    }
    assert (bytes == sz_msg);
    int ret = nn_shutdown (sock1, 1);//int how = 0 in original but returns error
    if (ret != 0) cout << "nn_shutdwon failed with error code " << nn_strerror(nn_errno());

    return ret;
}

int setupbm(const char *url, size_t bufsize, size_t repeatsfix, vector<size_t>messagsizes){

    int *mymsg = createbuf(bufsize);
    size_t repeats = repeatsfix;
    int factor = 2;
    for (size_t sz_msg = messagsizes.front(); sz_msg < messagsizes.back(); sz_msg = sz_msg * factor){
        if (sz_msg >= 8192){
            repeats = (repeatsfix*1000)/sz_msg;
         }
        if (repeats <= 1) repeats = 2;
        
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        send(url, mymsg, sz_msg, repeats);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        double time = time_span.count();
        
        cout << sz_msg*repeats << " " << repeats << " " << sz_msg << " " << time << " " << (sz_msg*repeats)/(time*1000000) << endl;
    }
    
    mymsg[0] = 0;
    send(url, mymsg, 4, 1);
    free(mymsg);

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



int main (const int argc, char **argv)
{
    Parameters params(argc, argv);
    const char *url = params.geturl();
    Type type = params.gettype();

    switch (type) {
        case server:{
            receive(url);
            break;
        }
        case client:{
            setupbm(url, params.getbuffersize(), params.getrepeats(), params.getmessagesizes());
        break;
        }
    default:
        fprintf (stderr, "Usage: pubsub %s|%s <URL> <ARG> ...\n",
                 SERVER, CLIENT);
        return 1;
    }
}
