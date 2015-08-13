#include "parameters.h"

Parameters::Parameters(int argc, char **argv){
    int opt;
    buffersize = 2147483648;
    repeats = 2000000;
    
    //-------------------------------------------------------------------------------------------
    
    static struct option longopts[] = {
        { "help",               no_argument,              NULL,	     'h' },
        { "buffer_size",            required_argument,	     NULL,       'b' },
        { "repeats",              required_argument,        NULL,       'r' },
        { NULL,	     0,			     NULL,	     0 }
    };
    
    while ((opt = getopt_long (argc, argv, "hb:r:", longopts, NULL)) != -1)
        switch (opt)
    {
        case 'h':
            std::cout << "------------------\nWelcome to this nanomsg Benchmark program\n you may choose the following options\n ---------------------\n --help                                     -h       to view this help tutorial \n\n" << std::endl;
            std::cout << "  -b      buffsize (DEFAULT = " << buffersize << std::endl;
            std::cout << "  -r      repeats (DEFAULT = " << repeats << std::endl;

            exit(1);
        case 'b':
            buffersize = atof(optarg);
            if (!(buffersize >= 0)) {
                printf("ERROR -b: please enter vaild buffersize (i.e. 2^n where n can be any natural number) \n");
                exit(1);
            }
            break;
        case 'r':
            repeats = atof(optarg);
            if (!(repeats >= 0)) {
                printf("ERROR -r: please enter vaild number for repeats \n");
                exit(1);
            }
            break;
        case '?':
            fprintf (stderr,
                     "ERROR: Unknown option character `\\x%x'.\n",
                     optopt);
        default:
            abort ();
    }

    
    std::cout << "#buffer size " << buffersize << " repeats " << repeats << std::endl;
}