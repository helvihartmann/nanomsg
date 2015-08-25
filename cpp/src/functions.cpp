#include <time.h>
#include <stdio.h>
#include <string.h>
#include <nanomsg/nn.h>
#include <nanomsg/survey.h>
#include <cstdlib>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <cassert>

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
    while (!end){

        int *buf = NULL;
        bytes = nn_recv (sock, &buf, NN_MSG, 0);
        checksum = checkbuf(buf, bytes);

        if (checksum != 1){
            end = true;
            if (checksum != 0) printf ("ERROR occured, received wrong numbers, checksum = %d\n", checksum);
        }
        nn_freemsg (buf);
    }
    nn_shutdown (sock, 0);
}

void send(int sock, size_t bufsize, size_t repeatsfix){
    
    int bytes = 0;
    size_t repeats = 0;
    struct timeval start, end;
    int factor = 2;
    size_t endsz_msg = 2<<26;
    size_t startsz_msg = 4;
    int *mymsg;
    mymsg = (int *) malloc(sizeof(*mymsg) * bufsize);
    
    for (int i = 0; i < bufsize; i++){
        mymsg[i] = 1;
    }
    std::cout << "Created buff of size " << bufsize << " Bytes" << std::endl;
    
    for (size_t sz_msg = startsz_msg; sz_msg < endsz_msg; sz_msg = sz_msg * factor){
        
        //repeats = repeats/2;
        repeats = repeatsfix;
        if (sz_msg >= 262144){ //1048576
            repeats = (repeatsfix*10000)/sz_msg;
        }
        if (repeats == 0) repeats = 5;
        
        gettimeofday(&start, NULL);
        for(int i = 0; i < repeats; i++){
            bytes = nn_send (sock, mymsg, sz_msg, 0);
            //sleep(1);
        }
        gettimeofday(&end, NULL);
        
        float time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
        std::cout << repeats*sz_msg << " " << repeats << " " << sz_msg << " " << time/1000000 << " " << (repeats*sz_msg)/time << std::endl;
        

    }
    
    mymsg[0] = 0;
    nn_send (sock, mymsg, 4, 0);
    nn_shutdown(sock, 0);
    free(mymsg);
}