#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <cstdlib>
#include <string>
#include "parameters.h"
#include "socketmng.h"
#include <assert.h>

pthread_t tid[2];

using namespace std;


int main(const int argc, char **argv){
    void *status;
    //Socketmng sock;
    Parameters params(argc, argv, proc_agent);
    int id = params.getid();
    
    int timeout = params.gettimeout();
    sleep(timeout);
    Agentservercomm server(id);
    Agentworkercomm worker(id);

    
    //--------------------------------
    assert(pthread_create(&(tid[1]), NULL, &Agentservercomm::heartbeat,&server) == 0);
    
    server.setup();///0

//WORK------------------------------------
    
    //Worker instructions------------------------------------
    sleep(1);
    
    worker.instruction(shm_connect, writer);
    worker.instruction(shm_fill, writer);
    worker.instruction(shm_xor_all, writer);
    worker.instruction(shm_connect, reader);
    worker.instruction(shm_xor_all, reader);
    worker.instruction(shm_disconnect, reader);
    worker.instruction(sock_close, reader);
    worker.instruction(shm_disconnect, writer);
    worker.instruction(shm_remove, writer);
    worker.instruction(sock_close, writer);

    //finish------------------------------------
    sleep(8);
    server.setfinishflag();
    cout << "AGENT: checkpoint" << endl;
    pthread_join(tid[1],&status);
    server.setup();//2

    worker.close();
    server.close();
    cout << "AGENT: finished" << endl;
    return 0;
}