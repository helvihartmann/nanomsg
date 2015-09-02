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
#include "socketmng.h"
#include <sys/time.h>
#include <errno.h>
#include <vector>
#include <limits>

using namespace std;
int checkbuf (const int *buf, int bytes);
void receive(int sock);
void send(int sock, size_t buffsize, size_t repeats);
int* createbuf(size_t bufsize);


string createurl(const char* plainurl, string port, string name){
    string url = plainurl + port + name;
    cout << url << endl;

    return url;
}

int syncservice_client(const char *url, Socketmng *socketmng){
    
    sleep(1);
    int sock1 = socketmng->open(url, req, connect);
     
    cout << "CLIENT: sending request" << endl;

    int msg = 1024;
    int bytes = nn_send (sock1, &msg, sizeof(int), 0);
    
    int *buf = NULL;
    bytes = nn_recv (sock1, &buf, NN_MSG, 0);
    nn_freemsg (buf);

    return sock1;
}

vector<int> syncservice_server(vector<string> urls, int nmbrofsubscribers, Socketmng *socketmng){
    int subscribers = 0;
    vector<int>sockets;
    while (subscribers < nmbrofsubscribers) {
        const char* url = urls.at(subscribers).c_str();
        int sock1 = socketmng->open(url, rep, bind);
        
        cout << "SERVER: waiting for request" << endl;

        int *buf = NULL;
        int bytes = nn_recv (sock1, &buf, NN_MSG, 0);
        nn_freemsg (buf);
        cout << "SERVER: received request" << endl;
        
        int msg = 1024;
        bytes = nn_send (sock1, &msg, sizeof(int), 0);
        if(bytes < 0) cout << "SERVER: ERROR replying, " << nn_strerror(nn_errno()) << endl;
        cout << "SERVER: replied" << endl;

        subscribers++;
        sockets.push_back(sock1);
    }
    return sockets;
}


int pubserver (const char *plainurl, Parameters *params, Socketmng *socketmng){
    
    int sock0 = socketmng->open(createurl(plainurl, ":555", "5").c_str(), pub, bind);
    
    int bytes = 0;
    vector<string>urls;
    for (int i = 0; i < params->getnmbrsubs(); i++){
        urls.push_back(createurl(plainurl, ":666", to_string(i)));
    }
    vector<int> sockets = syncservice_server(urls, params->getnmbrsubs(), socketmng);
    send(sock0, params->getbuffersize(), params->getrepeats());
    
    for (int i = 0; i < sockets.size(); i++){
        socketmng->close(sockets.at(i));
    }
}

int subclient (const char *plainurl, const char *name, Socketmng *socketmng){
    int sock0 = socketmng->open(createurl(plainurl, ":555", "5").c_str(), sub, connect);
    int sock1 = syncservice_client(createurl(plainurl, ":666", name).c_str(), socketmng);
    receive(sock0);
    
    socketmng->close(sock1);
}


int main (const int argc, char **argv){
    
    Parameters params(argc, argv);
    Socketmng socketmng;
    Type type = params.gettype();
    const char *url = params.geturl();
    switch (type) {
        case server:
            pubserver(url, &params, &socketmng);
            break;
        case client:
            subclient (url, params.getname(), &socketmng);
            break;
        default:
            fprintf (stderr, "Usage: pubsub %s|%s <URL> <ARG> ...\n",
                     SERVER, CLIENT);
            return 1;
            break;
    }
}