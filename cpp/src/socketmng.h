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

#include <stdbool.h>
#include <nanomsg/pair.h>
#include <nanomsg/tcp.h>
#include <iostream>


using namespace std;

enum Sock{pull, push, req, rep, pub, sub, survey, respond};
enum Sockconnect{connect, bind};

class Socketmng{
private:
    Sock socktype;
    Sockconnect sockconnect;
    
public:
    int open(const char *url, enum Sock socktype, enum Sockconnect sockconnect);
    int close (int sock);
};

