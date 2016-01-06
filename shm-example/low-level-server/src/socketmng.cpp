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
            
            if(instance->getagentid() == 1 && counter == 2) sleep(4);
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

Serveragentcomm::Serveragentcomm(int nmbr_agents_):
    nmbr_agents(nmbr_agents_)
{
    for(int i = 0; i < nmbr_agents; i++){
        sock_agent.push_back(open(createurl("tcp://*", ":", to_string(5000+i)).c_str(), req, binds));
    }
    sock_heartbeat = open("tcp://*:6666", survey, binds);
    hb_misses.assign(nmbr_agents,0);
}


void Serveragentcomm::setup(int msg){
    int *buf = NULL;
    
    for(int i = 0; i < nmbr_agents; i++){
        std::cout << "SERVER: exchanging message with agent " << i << std::endl;
        nn_send(sock_agent.at(i), &msg, 8, 0);
        nn_recv(sock_agent.at(i), &buf, NN_MSG, 0);
        cout << "SERVER: agent says " << *buf << endl;
    }
    nn_freemsg(buf);
}

void Serveragentcomm::checkanswers(vector<int>hb_answers, int respondants){
    sort(hb_answers.begin(), hb_answers.begin());
    
    cout << endl;
    for(unsigned int i = 0; i < hb_answers.size(); i++){
        cout << "Server answers are " << hb_answers.at(i) << endl;
    }
    
    if(respondants != nmbr_agents){
        for(unsigned int i = 0; i < hb_answers.size(); i++){
            hb_misses.at(i)++;
        }
    }
    else hb_misses.assign(nmbr_agents,0);
    
    cout << endl;
    for(unsigned int i = 0; i < hb_misses.size(); i++){
        cout << "Server hb misses " << hb_misses.at(i) << endl;
    }
    cout << endl;
    hb_answers.clear();
}

int Serveragentcomm::collectsurvey(){
    int *buf = NULL;
    int bytes = 0;
    int respondants = 0;
    vector<int>hb_answers;
    while (1) {
        bytes = nn_recv(sock_heartbeat, &buf, NN_MSG, 0);
        if(bytes == -1) break;
        if(*buf == 66){
            hb_finishers++;
            cout << " finishers " << hb_finishers << " from " << nmbr_agents << endl;
        }

        respondants++;
        hb_answers.push_back(*buf);
        *buf = 0;
    }
    if(hb_answers.size() == 0){
        cout << "SERVER: FATAL ERROR no heartbeat received at all" << endl;
        exit(1);
    }
    nn_freemsg(buf);
    
    if(hb_finishers >= nmbr_agents) return 0;
    checkanswers(hb_answers, respondants);
    return 1;
}

void Serveragentcomm::heartbeat(){
    int go = 1;
    int stop = 0;
    bool end = false;
    cout << "SERVER: sending first heartbeat " << go << endl;
    sleep(1);
    
    while(!end){
        
        nn_send(sock_heartbeat, &go, 8, 0);
        if(collectsurvey() == 0) end = true;
        go++;
    }
    
    nn_send(sock_heartbeat, &stop, 8, 0);
    cout << "SERVER: sending last heartbeat " << stop << endl;
    
    int *buf = NULL;
    nn_recv(sock_heartbeat, &buf, NN_MSG, 0);
    cout << "SERVER: received " << *buf << endl;
    nn_freemsg(buf);
}

void Serveragentcomm::close(){
    for(int i = 0; i < nmbr_agents; i++){
        Socketmng::close(sock_agent.at(i));
    }
    Socketmng::close(sock_heartbeat);
}
