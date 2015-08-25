#include <assert.h>
//#include <libc.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/survey.h>
#include "parameters.h"
#include <ctime>
#include <chrono>
#include <ratio>
#include <vector>

#define SERVER "server"
#define CLIENT "client"
#define DATE   "DATE"

using namespace std;
using namespace std::chrono;

int checkbuf (const int *buf, int bytes);
char *date();
void receive(int sock);
void send(int sock, size_t buffsize);

int surveyor(const char *url, vector<size_t> messagesizes){
    int sock = nn_socket (AF_SP, NN_SURVEYOR);
    
    int bytes = 0;
    int repeats = 4;
    int sz_d = 0;
    assert (sock >= 0);
    assert (nn_bind (sock, url) >= 0);
    sleep(1); // wait for connections
    
    size_t sz_msg = 0;
    size_t *test;
    test[0] = 0;
    for(int j = 0; j < messagesizes.size(); j++){
        
        sz_msg = messagesizes.at(j);
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        for(int i = 0; i < repeats; i++){

            bytes = nn_send (sock, &sz_msg, sizeof(size_t), 0);
            assert (bytes == sizeof(size_t));
            
            int checksum = 1;
            while (1){
                
                int *buf = NULL;
                bytes = nn_recv (sock, &buf, NN_MSG, 0);
                if (bytes == -1) break;
                
                //assert(bytes == sz_msg);
                //cout << "SEVER: received " << bytes << " bytes after requesting " << sz_msg << endl;
                nn_freemsg (buf);
            }
        }
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        double time = time_span.count();
        
        cout << sz_msg*repeats << " " << repeats << " " << sz_msg << " " << time << " " << (sz_msg*repeats)/(time*1000000) << endl;
    }
    
    test[0] = 1;
    bytes = nn_send(sock, test, sizeof(size_t), 0);
    cout << bytes << endl;
    return nn_shutdown (sock, 0);
    cout << bytes << endl;

}

int respondent (const char *url, const char *name, size_t bufsize){
    
    int bytes = 0;
    int sock = nn_socket (AF_SP, NN_RESPONDENT);
    assert (sock >= 0);
    assert (nn_connect (sock, url) >= 0);
    
    int *mymsg;
    mymsg = (int *) malloc(sizeof(*mymsg) * bufsize);
    
    for (int i = 0; i < bufsize; i++){
        mymsg[i] = 1;
    }
    std::cout << "Created buff of size " << bufsize << " Bytes" << std::endl;
    
    while(1){
        size_t *buf = NULL;
        bytes = nn_recv (sock, &buf, NN_MSG, 0);
        //if(strlen(buf) == 0) break;
        if(buf[0] == 1) break;
        if (bytes >= 0)
        {

            bytes = nn_send (sock, mymsg, *buf, 0);
            //assert (bytes == *buf);
            nn_freemsg (buf);
        }
    }
    cout << "CLIENT " << name << " terminating" << endl;
    return nn_shutdown (sock, 0);
}


int main (const int argc, char **argv)
{
    Parameters params(argc, argv);
    Type type = params.gettype();
    const char *url = params.geturl();
    switch (type) {
        case server:
            surveyor(url, params.getmessagesizes());
            break;
        case client:
            respondent (url, params.getname(), params.getbuffersize());
            break;
        default:
            fprintf (stderr, "Usage: pubsub %s|%s <URL> <ARG> ...\n",
                     SERVER, CLIENT);
            return 1;
            break;
    }
}