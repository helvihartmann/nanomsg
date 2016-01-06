#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include "socketmng.h"
#include <cstdlib>
#include <string>
#include <cstring>
#include "parameters.h"

using namespace std;


int main(const int argc, char **argv){

    Parameters params(argc, argv, proc_server);
    int nmbr_agents = params.getnmbragents();
    
    Serveragentcomm agent(nmbr_agents);
    

    
    sleep(2);//important to make sure agents are ready to receive requests
    agent.setup(2);
    

    agent.heartbeat();
    
    agent.setup(0);
    
    agent.close();
    cout <<"SERVER: finished" << endl;
    return 0;
}