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

enum Type{server, client, idler};
class Parameters{
private:
    size_t buffersize;
    size_t repeats;
    Type type;
    const char *url;
    const char *name;
    size_t sz_start, sz_end;
    int sz_factor;
    std::vector<size_t> messagesizes;
    int nmbrsubs;
    int cycles;
public:
    
    Parameters(int argc, char **argv);
    
    size_t getbuffersize() { return buffersize; }
    
    size_t getrepeats() { return repeats; }
    
    enum Type gettype() { return type; }
    
    const char * geturl() { return url; }
    
    const char *getname() { return name; }
    
    std::vector<size_t> getmessagesizes() { return messagesizes; }
    
    int getnmbrsubs() { return nmbrsubs; }
    
    int getcycles() { return cycles; }

};

#endif /*PARAMETERS_H*/