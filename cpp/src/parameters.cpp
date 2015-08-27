#include "parameters.h"

Parameters::Parameters(int argc, char **argv){
    sz_start = 4;
    buffersize = 2147483648; //8589934592; //1073741824; //2147483648;
    sz_factor = 2;
    int opt;
    repeats = 1000000;//2000000 good number for pushpull
    type = client;
    url = "";
    name = "";
    //-------------------------------------------------------------------------------------------
    
    static struct option longopts[] = {
        { "help",             no_argument,             NULL,	     'h' },
        { "buffersize",      required_argument,        NULL,       'b' },
        { "repeats",          required_argument,        NULL,       'r' },
        { "type",             required_argument,        NULL,       't' },
        { "url",              required_argument,        NULL,       'u' },
        { NULL,	     0,			     NULL,	     0 }
    };
    
    while ((opt = getopt_long (argc, argv, "hb:n:r:t:u:", longopts, NULL)) != -1)
        switch (opt)
    {
        case 'h':
            std::cout << "------------------\nWelcome to this nanomsg Benchmark program\n you may choose the following options\n ---------------------" << std::endl;
            std::cout << "--help        -h       to view this help tutorial" << std::endl;
            std::cout << "--buffersize  -b      buffsize (DEFAULT = " << buffersize << ")" << std::endl;
            std::cout << "--name        -n      name; enter any for your process to regonize " << std::endl;
            std::cout << "--repeats     -r      repeats (DEFAULT = " << repeats << ")" << std::endl;
            std::cout << "--type        -t      type: either choose server (0) or client (1) (DEFAULT = " << type << ")" << std::endl;
            std::cout << "--url         -u      url; Always enter url for process to connect. Program won't run without a given url" << std::endl;

            exit(1);
        case 'b':
            buffersize = atof(optarg);
            if (!(buffersize >= 0)) {
                printf("ERROR -b: please enter vaild buffersize (i.e. 2^n where n can be any natural number) \n");
                exit(1);
            }
            break;
        case 'n':
            name = optarg;
            break;
        case 'r':
            repeats = atof(optarg);
            if (!(repeats >= 0)) {
                printf("ERROR -r: please enter vaild number for repeats \n");
                exit(1);
            }
            break;
        case 't':
            if (strncmp (SERVER, optarg, strlen (SERVER)) == 0) type = server;
            else if (strncmp (CLIENT, optarg, strlen (CLIENT)) == 0) type = client;
            else if (strncmp (IDLER, optarg, strlen (IDLER)) == 0) type = idler;
            else {
                printf("ERROR -t: please enter vaild type: either server or client \n");
                exit(1);
            }
            break;
        case 'u':
            url = optarg;
            break;
        case '?':
            fprintf (stderr,
                     "ERROR: Unknown option character `\\x%x'.\n",
                     optopt);
        default:
            abort ();
    }

    if (strlen(url) == 0){
        std::cout << "ERROR: -u please enter valid url" << std::endl;
    }
    std::cout << "#type " << type << " " << name << " url " << url << "  buffer size " << buffersize << " repeats " << repeats << std::endl;
    
    sz_end = buffersize;
    if (sz_start <= sz_end){
        for (size_t p = sz_start; p <= sz_end; p = p * sz_factor){
            messagesizes.push_back(p);
        }
    }
    else{
        for (size_t p = sz_start; p >= sz_end; p = p/sz_factor){
            messagesizes.push_back(p);
        }
    }
}