#include <assert.h>
//#include <libc.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/survey.h>
#include <limits>
#include <vector>
#include <stdbool.h>
#include <nanomsg/pair.h>
#include <nanomsg/tcp.h>
#include <iostream>
#include <algorithm>

using namespace std;

enum Message{shm_connect, shm_disconnect, shm_fill, shm_xor_all, shm_remove, sock_close};
enum Sock{pull, push, req, rep, pub, sub, survey, respond};
enum Sockconnect{connect, binds};
enum Sockpartner{writer, reader, server};

class Socketmng{
private:
    Sock socktype;
    Sockconnect sockconnect;
public:
    string createurl(const char* plainurl, string port, string name);
    int open(const char *url, enum Sock socktype, enum Sockconnect sockconnect);
    int close (int sock);
};



class Agentworkercomm:Socketmng{
private:
    int sock_writer, sock_reader;
    
public:
    
    Agentworkercomm(int agent_id);
    void instruction(enum Message message, enum Sockpartner sockpartner);
    void close();
};



class Agentservercomm:Socketmng{
private:
    int agent_id;
    int sock_server, sock_heartbeat;
    bool finish_flag = false;
    
public:
    
    Agentservercomm(int agent_id);
    int getagentid() { return agent_id; }
    int comm(int sock, int msg);
    int setup();
    static void* heartbeat(void *arg);
    
    void setfinishflag() { finish_flag = true; }
    bool getfinishflag() { return finish_flag; }
    void close();
};



class Serveragentcomm:Socketmng{
private:
    int sock_heartbeat;
    vector<int> sock_agent;
    int nmbr_agents = 0;
    int hb_finishers = 0;
    vector<int>hb_misses;
public:
    
    Serveragentcomm(int nmbr_agents_);
    void checkanswers(vector<int>hb_answers, int respondants);
    int collectsurvey();
    void heartbeat();
    void setup(int msg);
    void close();
};