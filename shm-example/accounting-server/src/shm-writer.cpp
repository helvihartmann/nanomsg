#include "shmmanager.h"
#include "socketmng.h"
#include "parameters.h"

int main(const int argc, char **argv){
    Socketmng sock;
    Shmmanager shm;
    Parameters params(argc, argv, proc_writer);
    const char *url = params.geturl();

    
    int sock_agent = sock.open(url, rep, binds);
    int *buf = NULL;
    int bytes = 0;
    int ok = 1;
    
    while(1){
        //std::cout << "WRITER: waiting for agent" << std::endl;
        bytes = nn_recv(sock_agent, &buf, NN_MSG, 0);
        assert(bytes > 0);
        Message instr = static_cast<Message>(*buf);
        //std::cout << "WRITER: received from agent " << *buf << std::endl;

        switch (instr) {
            case shm_connect:
                shm.connect_write();
                break;
                
            case shm_disconnect:
                shm.disconnect();
                break;
                
            case shm_fill:
                shm.fill();
                break;
                
            case shm_xor_all:
                shm.xor_all();
                break;
                
            case shm_remove:
                shm.remove();
                break;
                
            case sock_close:
                nn_send(sock_agent, &ok, sizeof(ok), 0);
                sock.close(sock_agent);
                return 0;

            default:
                std::cout << "ERROR: writer receied no good instruction from agent" << std::endl;
                break;
        }
        
        nn_send(sock_agent, &ok, sizeof(ok), 0);
    }
}