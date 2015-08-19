#include <time.h>
#include <stdio.h>
#include <string.h>
#include <nanomsg/nn.h>
#include <nanomsg/survey.h>
#include <cstdlib>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>

int checkbuf (const int *buf, int bytes){
    int j = 0;
    int sum = 0;
    for ( int i = 0; i < bytes/4; i++){
        j = buf[i];
        //printf ("%d", j);
        sum = sum + j;
    }
    //printf ("\n");
    return sum/(bytes/sizeof(int));
}

char *date ()
{
    time_t raw = time (&raw);
    struct tm *info = localtime (&raw);
    char *text = asctime (info);
    text[strlen(text)-1] = '\0'; // remove '\n'
    return text;
}

void receive(int sock){
    int bytes = 0;
    int checksum = 0;
    bool end = false;
    while (!end)
    {
        std::cout << "SERVER: checkpoint 2" << std::endl;

        int *buf = NULL;
        bytes = nn_recv (sock, &buf, NN_MSG, 0);
        checksum = checkbuf(buf, bytes);
        std::cout << "SERVER: checkpoint 3" << std::endl;

        if (checksum != 1){
            end = true;
            if (checksum != 0) printf ("ERROR occured, received wrong numbers, checksum = %d\n", checksum);
        }
        std::cout << "SERVER: checkpoint 4, bytes " << bytes << std::endl;

        nn_freemsg (buf);
        std::cout << "SERVER: checkpoint 5" << std::endl;

    }
    nn_shutdown (sock, 0);
}

void send(int sock, size_t bufsize){
    
    int bytes = 0;
    struct timeval start, end;
    int factor = 2;
    size_t endsz_msg = 2<<20;
    size_t startsz_msg = 4;
    size_t repeats = 2;
    //size_t repeats = params->getrepeats();
    int *mymsg;
    mymsg = (int *) malloc(sizeof(*mymsg) * bufsize);
    
    for (int i = 0; i < bufsize; i++){
        mymsg[i] = 1;
    }
    std::cout << "Created buff of size " << bufsize << " Bytes" << std::endl;
    
    for (size_t sz_msg = startsz_msg; sz_msg < endsz_msg; sz_msg = sz_msg * factor){
        
        gettimeofday(&start, NULL);
        
        for(int i = 0; i < repeats; i++){
            bytes = nn_send (sock, mymsg, sz_msg, 0);
            //std::cout << bytes << std::endl;
            //assert (bytes == sz_msg);
            sleep(1);
        }
        gettimeofday(&end, NULL);
        
        float time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
        std::cout << repeats*sz_msg << " " << repeats << " " << sz_msg << " " << time/1000000 << " " << sz_msg/time << std::endl;
    }
    
    mymsg[0] = 0;
    nn_send (sock, mymsg, 4, 0);
    nn_shutdown(sock, 0);
    free(mymsg);
}