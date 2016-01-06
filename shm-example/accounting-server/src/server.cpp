#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include "socketmng.h"
#include <cstdlib>
#include <string>
#include <cstring>
#include "parameters.h"

using namespace std;


int main(const int argc, char **argv){

    int go = 1;
    int stop = 0;
    bool end = false;
    int answer = 0;
    Parameters params(argc, argv, proc_server);
    int nmbr_agents = 0;
    
    Serveragentcomm agent;

    

    while(nmbr_agents < 2){
        answer = agent.heartbeat(go);
        if(answer == 0) end = true;
        nmbr_agents = agent.getnmbrhbanswerers();
        go++;
    }
    agent.printhbanswerers(go);

    vector<int>agents;
    cout << "nmbr_agents " << nmbr_agents << endl;
    for(int i = 0; i < nmbr_agents; i++) agents.push_back(i);
    
    agent.createreq(nmbr_agents);
    agent.sendviareq(2,agents);
    

    while(!end){
        answer = agent.heartbeat(go);
        if(answer == 0) end = true;
        agent.printhbanswerers(go);
        go++;
    }
    agent.heartbeat(stop);
    
    for(int i = agents.size(); i < agent.getallagents(); i++) agents.push_back(i);
    agent.sendviareq(0,agents);
    agent.close();
    cout <<"SERVER: finished" << endl;
    return 0;
}
