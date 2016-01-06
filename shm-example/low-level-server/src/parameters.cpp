#include "parameters.h"

Parameters::Parameters(int argc, char **argv, enum Processtype process_):
    process(process_)
{
    int opt;
    proc_id = 0;
    nmbr_agents = 0;
    url = "";

    //-------------------------------------------------------------------------------------------
    
    static struct option longopts[] = {
        { "help",             no_argument,             NULL,	     'h' },
        { "sock_id",            required_argument,       NULL,      'i' },
        { "url",              required_argument,        NULL,       'u' },
        { NULL,	     0,			     NULL,	     0 }
    };
    
    while ((opt = getopt_long (argc, argv, "hi:n:u:", longopts, NULL)) != -1)
        switch (opt)
    {
        case 'h':
            std::cout << "------------------\nWelcome to this nanomsg Benchmark program\n you may choose the following options\n ---------------------" << std::endl;
            std::cout << "--help        -h       to view this help tutorial" << std::endl;
            std::cout << "--sock_id     -i      process id" << std::endl;
            std::cout << "--nmbr_agents -n      number of agents" << std::endl;
            std::cout << "--url         -u      url" << std::endl;

            exit(1);
        case 'i':
            proc_id = atoi(optarg);
                switch (process) {
                    case proc_agent:
                            if (!(proc_id > 0)) {
                                cout << "ERROR -i: please enter vaild process id for " << proc_id << endl;
                                exit(1);
                            }
                        break;
                    default:
                        if (!(proc_id >= 0)) {
                            cout << "ERROR -i: please enter vaild process id for " << proc_id << endl;
                            exit(1);
                        }
                        break;
                }
            break;
        case 'n':
            nmbr_agents = atoi(optarg);
            if (!(nmbr_agents >= 0)) {
                printf("ERROR -n: please enter vaild number of agents\n");
                exit(1);
            }
            break;

        case 'u':
            url = optarg;
            switch (process) {
                case proc_writer:
                case proc_reader:
                    if (strlen(url) == 0){
                        cout << "ERROR: -u please enter valid url" << endl;
                        exit(1);
                    }
                    break;
                case proc_server:
                case proc_agent:
                    break;
            }
            break;

        case '?':
            fprintf (stderr,
                     "ERROR: Unknown option character `\\x%x'.\n",
                     optopt);
        default:
            abort ();
    }

    
    
    std::cout << "#id " << proc_id << " number of agents I am talking to " << nmbr_agents << " url " << url << std::endl;
    
}