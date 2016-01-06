#include "socketmng.h"

string Socketmng::createurl(const char* plainurl, string port, string name){
    string url = plainurl + port + name;
    //cout << url << endl;
    
    return url;
}

int Socketmng::open(const char *url, enum Sock socktype, enum Sockconnect sockconnect){
    int sock = -1;
    int timeout = 1000;
    int sockopt = 0;
    switch (socktype) {
            case pull:
                sock = nn_socket (AF_SP, NN_PULL);
            break;
            case push:
                sock = nn_socket (AF_SP, NN_PUSH);
            break;
            case req:
                sock = nn_socket (AF_SP, NN_REQ);
            break;
            case rep:
                sock = nn_socket (AF_SP, NN_REP);
            break;
            case pub:
                sock = nn_socket (AF_SP, NN_PUB);
            break;
            case sub:
                sock = nn_socket (AF_SP, NN_SUB);
            break;
            case survey:
                sock = nn_socket (AF_SP, NN_SURVEYOR);
                sockopt = nn_setsockopt (sock, NN_SURVEYOR, NN_SURVEYOR_DEADLINE, &timeout, sizeof(timeout));
            break;
            case respond:
                sock = nn_socket (AF_SP, NN_RESPONDENT);
            break;
        default:
            break;
    }
    
    if (sock < 0){
        cout << "ERROR: nn_socket " << socktype << " failed with error code " << nn_strerror(nn_errno());
        exit(1);
    }
    
    int nnbufsize = numeric_limits<int>::max();
    //cout << nnbufsize << endl;
    sockopt = nn_setsockopt (sock, NN_SOL_SOCKET, NN_SNDBUF, &nnbufsize, sizeof(nnbufsize));
    if (sockopt < 0){
        cout << "ERROR: nn_setsockopt failed with error code " << nn_strerror(nn_errno());
        exit(1);
    }
    
    int connectnn, bindnn;
    switch (sockconnect) {
            case connect:
                connectnn = nn_connect (sock, url);
            if (connectnn < 0){
                cout << "ERROR: nn_connect failed with error code " << nn_strerror(nn_errno()) << endl;
                exit(1);
            }

            break;
            case binds:
                bindnn = nn_bind (sock, url);
            if (bindnn < 0){
                cout << "ERROR: nn_bind failed with error code " << nn_strerror(nn_errno()) << endl;
                exit(1);
            }

            break;
        default:
            break;
    }
    return sock;
}


int Socketmng::close (int sock){
    
    int ret = nn_shutdown (sock, 1);//int how = 0 in original but returns error
    if (ret != 0) cout << "nn_shutdwon failed with error code " << ret << ": " << nn_strerror(nn_errno());
    return ret;
}


/*----------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------*/
Agentworkercomm::Agentworkercomm(int agent_id){
    
    string writerurl = "ipc://writer" + to_string(agent_id);
    string writerstart = "./../build/shm-writer -u " + writerurl + " &";
    string readerurl = "ipc://reder" + to_string(agent_id);
    string readerrstart = "./../build/shm-reader -u " + readerurl + " &";
    
    //system("./../build/shm-writer -u ipc://<service><id> &");
    int sysstatus = system(writerstart.c_str());
    if(sysstatus != 0) cout << "Error: opening writer returned " << sysstatus << endl;
    sysstatus = system(readerrstart.c_str());
    if(sysstatus != 0) cout << "Error: opening reader returned " << sysstatus << endl;
    
    sock_writer = open(writerurl.c_str(), req, connect);
    sock_reader = open(readerurl.c_str(), req, connect);
}


void Agentworkercomm::instruction(enum Message message, enum Sockpartner sockpartner){
    int *buf = NULL;
    int sock;
    
    switch (sockpartner) {
        case writer:
            sock = sock_writer;
            break;
        case reader:
            sock = sock_reader;
            break;
        default:
            cout << "ERROR in agentworkercomm::instruction, wrong sockpartner" << sockpartner << endl;
            exit(0);
    }
    nn_send(sock, &message, sizeof(message), 0);
    nn_recv(sock, &buf, NN_MSG, 0);
    //cout << "AGENT: worker says " << *buf << endl;
    
    nn_freemsg(buf);
}

void Agentworkercomm::close(){
    
    Socketmng::close(sock_writer);
    Socketmng::close(sock_reader);
}


/*----------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------*/
Agentservercomm::Agentservercomm(int agent_id_):
    agent_id(agent_id_)
{
    sock_server = open(createurl("tcp://node0", ":", to_string(5000+agent_id-1)).c_str(), rep, connect);
    sock_heartbeat = open("tcp://node0:6666", respond, connect);
}


int Agentservercomm::setup(){
    return comm(0,1);
}


int Agentservercomm::comm(int whichsock, int msg){
    int sock;
    //int msg 0 for stop and 1 for ok (continue)
    
    if(whichsock == 0) sock = sock_server;
    else if(whichsock == 1) sock = sock_heartbeat;
    else{
       exit(1);
       cout << "AGENT: Error in communication, name the correct socket you want to use. sock was " << whichsock << endl;
    }
    
    int answer = 0;
    int *buf = NULL;
    //SERVER------------------------------------
    nn_recv(sock, &buf, NN_MSG, 0);
    answer = *buf;
    cout <<"AGENT: received survey " << answer << " and sends " << msg << endl;
    nn_send(sock, &msg, sizeof(int), 0);
    
    nn_freemsg(buf);
    return answer;
}

void* Agentservercomm::heartbeat(void *arg){
    Agentservercomm* instance = static_cast<Agentservercomm*>(arg);
    
    cout << "Agent " << instance->getagentid() << " thread connecting " << endl;
    
    
    int counter = 0;
    int answer = 0;
    bool end = false;
    int msg = instance->getagentid();//1 for ok
    cout << msg << endl;
    while (!end) {
        if(instance->getfinishflag()==1){
            cout << "Socketmanager: flag is at " << instance->getfinishflag() << " I will send 66" << endl;
            instance->comm(1,66);//send fnish
            instance->comm(1,0);//confirm
            end = true;
        }
        else{
            answer = instance->comm(1,msg);//1
            
            if(instance->getagentid() == 1 && counter == 5) sleep(4);
            if(answer == 0) end = true;
            counter++;
        }
        
        instance->getfinishflag();
    }
    return NULL;
}

void Agentservercomm::close(){
    Socketmng::close(sock_heartbeat);
    Socketmng::close(sock_server);
}

/*----------------------------------------------------------------------------------------
 --------------------------------------------------------------------------------------*/

Serveragentcomm::Serveragentcomm(){
    sock_heartbeat = open("tcp://*:6666", survey, binds);
}

void Serveragentcomm::createreq(int nmbr_agents_){
    nmbr_agents = nmbr_agents_;
    for(unsigned int i = sock_agent.size(); i < nmbr_agents; i++){
        sock_agent.push_back(open(createurl("tcp://*", ":", to_string(5000+i)).c_str(), req, binds));
        cout << "SERVER: binding to " << createurl("tcp://*", ":", to_string(5000+i)) << endl;
    }
}


void Serveragentcomm::sendviareq(int msg, vector<int> agents){
    int *buf = NULL;
    int j = 0;
    for(unsigned int i = 0; i < agents.size(); i++){
        j = agents.at(i);
        cout << "SERVER: exchanging message with agent " << j+1 << " size sock agents " << sock_agent.size() << endl;
        nn_send(sock_agent.at(j), &msg, 8, 0);
        nn_recv(sock_agent.at(j), &buf, NN_MSG, 0);
        cout << "SERVER: agent says " << *buf << endl;
        fullagentlist.push_back(j);
    }
    nn_freemsg(buf);
}

int Serveragentcomm::printhbanswerers(int idx_hb){
    if(hb_finishers >= fullagentlist.size()){
        cout << "SERVER: all agents finished" << endl;
        return 0;
    }
    cout << endl;
    cout << "SERVER: Hb " << idx_hb << " answered by agents " << hb_answers.size() << ": ";
    for(unsigned int i = 0; i < hb_answers.size(); i++){
        cout << hb_answers.at(i) << " ";
    }
    cout << endl;
    return 1;
}

void Serveragentcomm::printhbmisses(int idx_hb){
    cout << "\n\n" << endl;
    cout << "SERVER: hb " << idx_hb << ": agents missing the hb are ";
    for(unsigned int i = 0; i < hb_misses.size(); i++){
        if(hb_misses.at(i) != 0){
            total_hb_misses.at(i)++;

            cout << i+1 << " with " << total_hb_misses.at(i) << " misses & ";
        }
    }
    cout << " - ";
}

void Serveragentcomm::addnewagent(){
    vector<int>agents;
    for(unsigned int i = nmbr_agents; i < hb_answers.size(); i++){
        nmbr_agents++;
        createreq(nmbr_agents);
        agents.push_back(i);
    }
    sendviareq(2,agents);
}

int Serveragentcomm::checkanswers(int idx_hb){
    //find out who didnt answer and add to hb_misses.at(i)
    int sum_of_elems = 0;
    int j = 0;
    int respondants = 0;
    
    if(nmbr_agents != 0){
        if(hb_answers.size() > nmbr_agents) addnewagent();
        respondants = nmbr_agents;
    }
    else respondants = hb_answers.size();
    total_hb_misses.resize(respondants);
    hb_misses.assign(respondants,1);
    
    /*if(nmbr_agents != 0){
     if(hb_answers.size() > nmbr_agents) addnewagent();
     respondants = nmbr_agents - hb_finishers;
     total_hb_misses.resize(fullagentlist.size());
     }
     else{
     respondants = hb_answers.size();
     total_hb_misses.resize(respondants);
     }
     cout << respondants << " , " << total_hb_misses.size() << endl;*/
    
    for(unsigned int i = 0; i < hb_answers.size(); i++){
        j = hb_answers.at(i)-1;//find out who answered
        if(j >= 0 && j != 65) hb_misses.at(j)--;
    }

    for (int n : hb_misses)sum_of_elems += n;
    if(sum_of_elems != 0)printhbmisses(idx_hb);
    else total_hb_misses.assign(respondants,0);
    return respondants;
}

int Serveragentcomm::collectsurvey(int idx_hb){
    hb_answers.clear();
    int *buf = NULL;
    int bytes = 0;
    while (1) {
        bytes = nn_recv(sock_heartbeat, &buf, NN_MSG, 0);//check buf size
        if(bytes == -1) break;//timeout
        assert(bytes == sizeof(int));
        if(*buf == 0) break;
        if(*buf == 66) hb_finishers++;
        hb_answers.push_back(*buf);
        

        *buf = 0;
    }
    if(hb_answers.size() == 0){
        cout << "SERVER: ERROR no heartbeat received at all" << endl;
        return 1;
    }
    else     sort(hb_answers.begin(), hb_answers.end());


    nn_freemsg(buf);
    unsigned int respondants = checkanswers(idx_hb);
    if(hb_finishers >= fullagentlist.size() && hb_finishers >= respondants) return 0;
    return 1;
}

int Serveragentcomm::heartbeat(int idx_hb){
    
    nn_send(sock_heartbeat, &idx_hb, 8, 0);
    int answer = collectsurvey(idx_hb);
    
    return answer;
}

void Serveragentcomm::close(){
    for(unsigned int i = 0; i < nmbr_agents; i++){
        Socketmng::close(sock_agent.at(i));
    }
    Socketmng::close(sock_heartbeat);
}
