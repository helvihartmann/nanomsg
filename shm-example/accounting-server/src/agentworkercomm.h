#include <iostream>
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/pubsub.h>
#include <nanomsg/survey.h>
#include "enumtable.h"

using namespace std;


class Agentworkercomm{
private:
    int sock_writer, sock_reader;
    
public:
    
    Agentworkercomm(int agent_id);
    void instruction(enum Message message, enum Socktype socktype);
    void close();
};