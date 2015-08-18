#ifndef PARAMETERS_H
#define PARAMETERS_H
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <string.h>

#define SERVER "server"
#define CLIENT "client"
/*2015 Copyright Helvi Hartmann <hhartmann@fias.uni-frankfurt.de>
 Parameter class takes in all options parsed in function call or sets default values*/

enum Type{server, client};
class Parameters{
private:
    size_t buffersize;
    size_t repeats;
    Type type;
    const char *url;
    const char *name;
public:
    
    Parameters(int argc, char **argv);
    
    size_t getbuffersize() { return buffersize; }
    
    size_t getrepeats() { return repeats; }
    
    enum Type gettype() { return type; }
    
    const char * geturl() { return url; }
    
    const char *getname() { return name; }

};

#endif /*PARAMETERS_H*/