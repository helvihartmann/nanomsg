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
int* createbuf(size_t bufsize);
int close (int sock);
int open(const char *url, const char *socktype);
string createurl(const char* plainurl, string port, string name);

void socketssend(vector<int>sockets, size_t nmbr_sockets, int* mymsg, size_t sz_msg, size_t bufsize, int cycles){
    //cout << "SERVER: sending "  << sz_msg << " bytes " << repeats << " times" << endl;
    int end = 0;
    size_t bytes, index;
    for (size_t j = 0; j < nmbr_sockets; j++){
        for(size_t i = 0; i < cycles; i++){
            index = (sz_msg*j)%bufsize;
            //cout << "SERVER: sending to " << j << " "  << bytes << " bytes" << endl;
            bytes = nn_send (sockets.at(j), (mymsg+index), sz_msg, 0);
        }
    }
    if(sockets.size() > nmbr_sockets){
        bytes = nn_send (sockets.at(nmbr_sockets), &end, 4, 0);
    }
    
}

void socketsreceive(vector<int>sockets, int cycles){
    int bytes = 0;
    int checksum = 0;
    bool end = false;
    while(!end){
        for(size_t j = 0; j < sockets.size(); j++){
            int *buf = NULL;
            //cout << "CLIENT: waiting for " << j << endl;
            
            for(size_t i = 0; i < cycles; i++){
                bytes = nn_recv (sockets.at(j), &buf, NN_MSG, 0);

                //cout << "CLIENT: received from: " << j << " " << bytes << " bytes " << buf[0] << endl;
                
                assert(bytes>=0);
                if(buf[0] == 0){
                    //cout << "CLIENT: closing package size" << endl;
                    i = cycles;
                    j = sockets.size();
                }
                if(buf[0] == 2){
                    //cout << "CLIENT: closing all" << endl;
                    i = cycles;
                    j = sockets.size();
                    end = true;
                }
            
            }
            nn_freemsg (buf);
                
        }
    }

}

void serverpush(const char *plainurl, size_t bufsize, size_t socketsmax, vector<size_t>messagsizes, Socketmng *socketmng, int cycles){
    
    //create sockets and buffer----------------------------------------------------------------------------
    vector<int>sockets;
    int sock1 = 0;
    for(size_t i = 0; i < socketsmax; i++){
        sock1 = socketmng->open(createurl(plainurl, ":", to_string(5000+i)).c_str(), push, connect);
        sockets.push_back(sock1);
    }
    
    int *mymsg = createbuf(bufsize);
    
    //loop over package size----------------------------------------------------------------------------
    int factor = 2;
    size_t nmbr_sockets = socketsmax;
    for (size_t sz_msg = messagsizes.front(); sz_msg < messagsizes.back(); sz_msg = sz_msg * factor){
    //for (size_t sz_msg = messagsizes.back(); sz_msg > messagsizes.front(); sz_msg = sz_msg / factor){
        if (sz_msg >= 8192){
            nmbr_sockets = (socketsmax*3000)/sz_msg;
        }
        if (nmbr_sockets <= 10) nmbr_sockets = 10;
        

        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        socketssend(sockets, nmbr_sockets, mymsg, sz_msg, bufsize, cycles);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
    
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        double time = time_span.count();
        
        size_t iterations = nmbr_sockets * cycles;
        cout << sz_msg*iterations << " " << iterations << " " << sz_msg << " " << time << " " << (sz_msg*iterations)/(time*1000000) << endl;
    }
    int end = 2;
    sleep(5);
    cout << "SERVER: closing client sockets" << endl;
    nn_send (sockets.at(0), &end, 4, 0);
    for(size_t i = 0; i < sockets.size(); i++){
        socketmng->close(sockets.at(i));
    }
    
    free(mymsg);
}


void clientpull(const char *plainurl, Socketmng *socketmng, size_t socketsmax, int cycles){
    
    vector<int>sockets;
    for(size_t i = 0; i < socketsmax; i++){
        sockets.push_back(socketmng->open(createurl(plainurl, ":", to_string(5000+i)).c_str(), pull, bind));
    }
    
    socketsreceive(sockets, cycles);

    for(size_t i = 0; i < sockets.size(); i++){
        socketmng->close(sockets.at(i));
    }
}



int main (const int argc, char **argv)
{
    Parameters params(argc, argv);
    Socketmng sockets;
    size_t repeats = params.getrepeats();
    const char *url = params.geturl();
    Type type = params.gettype();
    int cycles = 2;
    switch (type) {
        case server:{
            clientpull(url, &sockets, repeats, cycles);
            break;
        }
        case client:{
            serverpush(url, params.getbuffersize(), repeats, params.getmessagesizes(), &sockets, cycles);
        break;
        }
    default:
        fprintf (stderr, "Usage: pubsub %s|%s <URL> <ARG> ...\n",
                 SERVER, CLIENT);
        return 1;
    }
    return 0;
}
