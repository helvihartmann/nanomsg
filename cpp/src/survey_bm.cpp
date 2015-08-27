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
int* createbuf(size_t bufsize);

//char *date();
//void receive(int sock);
//void send(int sock, size_t buffsize);

int collectsurvey(int sock){
    int checksum = 1;
    int messagesreceived = 0;
    int bytes = 0;
    while (1){
        
        int *buf = NULL;
        bytes = nn_recv (sock, &buf, NN_MSG, 0);
        if (bytes == -1) break;
        
        //assert(bytes == sz_msg);
        checksum = checkbuf(buf, bytes);
        if (checksum != 1){
            if (checksum != 0) printf ("ERROR occured, received wrong numbers, checksum = %d\n", checksum);
            break;
        }
        
        nn_freemsg (buf);
        messagesreceived++;
    }
    return messagesreceived;
}


int surveyor(const char *url, vector<size_t> messagesizes){
    int sock = nn_socket (AF_SP, NN_SURVEYOR);
    assert (sock >= 0);
    int timeout = 2000;
    int sockopt = nn_setsockopt (sock, NN_SURVEYOR, NN_SURVEYOR_DEADLINE, &timeout, sizeof(timeout));
    assert(sockopt >= 0);

    assert (nn_bind (sock, url) >= 0);
    sleep(1); // wait for connections
    
    int bytes = 0;
    int repeats = 2;
    int sz_d = 0;
    size_t sz_msg = 0;
    size_t *test;
    test[0] = 0;
    for(int j = 0; j < messagesizes.size(); j++){
        
        int messagesreceived = 0;
        sz_msg = messagesizes.at(j);
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        for(int i = 0; i < repeats; i++){

            //startsurvey
            bytes = nn_send (sock, &sz_msg, sizeof(size_t), 0);
            assert (bytes == sizeof(size_t));
    
            messagesreceived = collectsurvey(sock);
        }
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        double time = time_span.count();
        
        if(messagesreceived == 0){
            cout << sz_msg*repeats << " " << repeats << " " << sz_msg << "  " << time << " - " << endl;
        }
        else{
            cout << sz_msg*repeats << " " << repeats << " " << sz_msg << " " << time << " " << (sz_msg*repeats)/(time*1000000) << " - " << messagesreceived << endl;
        }
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
    
    int *mymsg = createbuf(bufsize);
    
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

int idle (const char *url, const char *name){
    
    int bytes = 0;
    int sock = nn_socket (AF_SP, NN_RESPONDENT);
    assert (sock >= 0);
    assert (nn_connect (sock, url) >= 0);
    
    while(1){
        size_t *buf = NULL;
        bytes = nn_recv (sock, &buf, NN_MSG, 0);
        if(buf[0] == 1) break;
    }
    cout << "IDLER " << name << " terminating" << endl;
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
        case idler:
            idle (url, params.getname());
            break;

        default:
            fprintf (stderr, "Usage: pubsub %s|%s <URL> <ARG> ...\n",
                     SERVER, CLIENT);
            return 1;
            break;
    }
}