#include "socketmng.h"

int Socketmng::open(const char *url, enum Sock socktype, enum Sockconnect sockconnect){
    int sock;
    int timeout = 3000;
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
                cout << "ERROR: nn_connect failed with error code " << nn_strerror(nn_errno());
                exit(1);
            }

            break;
            case bind:
                bindnn = nn_bind (sock, url);
            if (bindnn < 0){
                cout << "ERROR: nn_bind failed with error code " << nn_strerror(nn_errno());
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
