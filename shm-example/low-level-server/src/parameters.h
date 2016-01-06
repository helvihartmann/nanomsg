#ifndef PARAMETERS_H
#define PARAMETERS_H
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <string.h>
#include <vector>

#define SERVER "server"
#define CLIENT "client"
#define IDLER "idler"
/*2015 Copyright Helvi Hartmann <hhartmann@fias.uni-frankfurt.de>
 Parameter class takes in all options parsed in function call or sets default values*/

using namespace std;
enum Processtype{proc_server, proc_agent, proc_reader, proc_writer};

class Parameters{
private:
    int proc_id;
    int nmbr_agents;
    const char *url;
    enum Processtype process;
public:
    
    Parameters(int argc, char **argv, enum Processtype process_);
    
    size_t getid() { return proc_id; }
    
    int getnmbragents() { return nmbr_agents; }
    
    const char * geturl() { return url; }

};

#endif /*PARAMETERS_H*/