#ifndef PARAMETERS_H
#define PARAMETERS_H
#include <unistd.h>
#include <getopt.h>
#include <iostream>
/*2015 Copyright Helvi Hartmann <hhartmann@fias.uni-frankfurt.de>
 Parameter class takes in all options parsed in function call or sets default values*/

class Parameters{
private:
    size_t buffersize;
    size_t repeats;
public:
    
    Parameters(int argc, char **argv);
    
    size_t getbuffersize() { return buffersize; }
    
    size_t getrepeats() { return repeats; }

};

#endif /*PARAMETERS_H*/