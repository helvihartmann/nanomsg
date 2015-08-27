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
#include <ctime>
#include <chrono>
#include <stdlib.h>

using namespace std;
using namespace std::chrono;

int* createbuf(size_t bufsize){
    int *mymsg;
    mymsg = (int *) malloc(sizeof(*mymsg) * bufsize);
    
    for (int i = 0; i < bufsize; i++){
        mymsg[i] = rand();
    }
    cout << "Created buff of size " << bufsize << " Bytes" << endl;
    return mymsg;
}

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
            if (checksum != 0) cout << "ERROR occured, received wrong numbers, checksum = " << checksum << endl;
        }
        nn_freemsg (buf);
    }
    nn_shutdown (sock, 0);
}

void send(int sock, size_t bufsize, size_t repeatsfix){
    
    int bytes = 0;
    size_t repeats = 0;
    int factor = 2;
    size_t endsz_msg = 2<<26;
    size_t startsz_msg = 4;
    int *mymsg = createbuf(bufsize);
    
    for (size_t sz_msg = startsz_msg; sz_msg < endsz_msg; sz_msg = sz_msg * factor){
        
        //repeats = repeats/2;
        repeats = repeatsfix;
        if (sz_msg >= 262144){ //1048576
            repeats = (repeatsfix*10000)/sz_msg;
        }
        if (repeats == 0) repeats = 5;
        
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        for(int i = 0; i < repeats; i++){
            bytes = nn_send (sock, mymsg, sz_msg, 0);
            //sleep(1);
        }
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        double time = time_span.count();
        cout << sz_msg*repeats << " " << repeats << " " << sz_msg << " " << time << " " << (sz_msg*repeats)/(time*1000000) << endl;
    }
    
    mymsg[0] = 0;
    nn_send (sock, mymsg, 4, 0);
    nn_shutdown(sock, 0);
    free(mymsg);
}