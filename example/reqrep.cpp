#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <cassert>
#include "nanomsg/nn.h"
#include "nanomsg/reqrep.h"
#include "nanomsg/pipeline.h"
using namespace std;

#define ADDRESS_REQ "ipc/oauth2"
#define ADDRESS_REP "ipc/oauth2handler"
#define NO_THREADS 4
#define CLIENTS 2
pthread_t tid[NO_THREADS];

/************************************************************************************************

Request , nn_device and Reply endpoints are defined as below:

   Requests from webServer                       nn_device                                            REP end point
nn_connect[REQ(ADDRESS_REQ)] <--> nn_device( nn_bind[REP(ADDRESS_REQ)] , nn_bind[REQ(ADDRESS_REP)] )  <--> nn-connect[REP(ADDRESS_REP)]

I have also tried with the following. 
nn_connect[REQ(ADDRESS_REQ)] <--> nn_device( nn_bind[REP(ADDRESS_REQ)] , nn_connect[REQ(ADDRESS_REP)] )  <---> nn_bind[REP(ADDRESS_REP)]

************************************************************************************************/
void* device(void *arg);
void* reply_worker_pool(void *arg);
void* client_request(void *arg);

int main(int argc, const char * argv[]){
    int no_threads = 1;//1
    
    //assert(pthread_create(&(tid[0]), NULL, &device, NULL) == 0);
    
    while(no_threads < NO_THREADS){
        if(no_threads <= CLIENTS){
            cout << "creating clients " << no_threads << endl;
            assert(pthread_create(&(tid[no_threads]), NULL, &client_request, NULL) == 0);
        }
        else{
            cout << "creating worker " << no_threads << endl;
            assert(pthread_create(&(tid[no_threads]), NULL, &reply_worker_pool, NULL) == 0);

        }
        no_threads++;
    }
    
    int *buf = NULL;
    int endsock = nn_socket(AF_SP, NN_PULL);
    nn_bind(endsock, "ipc://end");
    nn_recv(endsock, &buf, 4, 0);

    return 0;
}


void* device(void *arg){
    
    int rc, device_rep, device_req;
    pthread_t id = pthread_self();
    
    //  Intialise the device sockets.
    
    device_rep = nn_socket (AF_SP_RAW, NN_REP);
    nn_bind (device_rep, "ipc://oauth2");
    
    device_req = nn_socket (AF_SP_RAW, NN_REQ);
    nn_connect (device_req, "ipc://oauth2handler");

    //  Run the device.
    rc = nn_device (device_req, device_rep);
    
    cout << "oauth2_device thread id: " << id << endl;

    // this means nn_device loop exited either by an error or by calling nn_term ()
    assert (rc < 0 && nn_errno () == ETERM);
    
    
    //  Clean up.
    
    //nn_close (device_rep);
    //nn_close (device_req);
    return NULL;
}


void* reply_worker_pool(void *arg){
    pthread_t id = pthread_self();
    
    int sock = nn_socket(AF_SP, NN_REP);
    assert(sock >= 0);
    assert( nn_bind(sock, "ipc://oauth2") >= 0 );
    
    cout << "send_authentication_request thread id: " << id << endl;
    
    int client = 0;
    while(client < CLIENTS)
    {
        int *buf = NULL;
        int bytes = nn_recv (sock, &buf, NN_MSG, 0);
        
        cout << "WORKER: thread id: " << id << " received: " << *buf << ", " << bytes << " bytes" << endl;
        
        if(bytes > 0){
            cout << "WORKER: replying" << endl;
            bytes = nn_send(sock, &client, 4, 0);
            if(bytes != 4) cout << "ERROR: nn_send " << bytes << " failed with error code " << nn_strerror(nn_errno()) << endl;
            nn_freemsg (buf);
        }
        client ++;
    }
    int end = 0;
    int endsock = nn_socket(AF_SP, NN_PUSH);
    nn_connect(endsock, "ipc://end");
    nn_send(endsock, &end, 4, 0);
    
    return NULL;
}

void* client_request(void *arg){
    int client_req = nn_socket (AF_SP, NN_REQ);
    assert(nn_connect (client_req, "ipc://oauth2") >= 0);
    cout << "CLIENT: connected to server reply socket" << endl;

    int *buf = NULL;
    int bytes = 0;
    int req = 0;
    bytes = nn_send(client_req, &req, 4, 0);
    bytes = nn_recv(client_req, &buf, NN_MSG,0);
    
    cout << "CLIENT: received reply: " << *buf << ", " << bytes << " bytes" << endl;
    if(bytes != 4) cout << "ERROR: nn_recv " << bytes << " failed with error code " << nn_strerror(nn_errno()) << endl;
    nn_freemsg (buf);

    
    return NULL;
}