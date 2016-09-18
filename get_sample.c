#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
/* Shared memory implementation */
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "collect_data.h"

void print_values(struct filtered_data filteredData){

    int i;
    long sec_elapsed;
    int ms_elapsed;
    int us_elapsed;
    double  filtered_channel[4];


    /* assign the values */
    sec_elapsed = filteredData.sec_elapsed;
    ms_elapsed = filteredData.ms_elapsed;
    us_elapsed = filteredData.us_elapsed;
    for(i=0; i < 4; i++) filtered_channel[i] = filteredData.channels[i];

    /* print values */
    printf("%ld,%d,%d,%f,%f,%f,%f\n",
               sec_elapsed,
               ms_elapsed,
               us_elapsed,
               filtered_channel[0],
               filtered_channel[1],
               filtered_channel[2],
               filtered_channel[3]);
}

void main(int argc, char* argv[])
{
    int i, j;

    struct filtered_data filteredData;
    struct shared* data_ptr = (struct shared*) NULL;

    int shmid;
    
    /* create a shared memory (CANNOT CREATE)*/
    if((shmid = shm_open(SHARED_RESOURCE, O_RDWR , 0600)) == -1)
    {perror("shm_open"); return;}
    ftruncate(shmid, sizeof(struct shared));

    /* attach to the shared memory (CAN READ ONLY)*/
    data_ptr = (struct shared *) mmap(0, sizeof(struct shared),
                PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
    if(data_ptr == (struct shared*) (-1)) {perror("mmap"); return;}

    /*******************************************
    *  CRITICAL SECTION BEGINS
    ********************************************/
    /* acquire MUTEX for critical section */
    pthread_mutex_lock(&(data_ptr->mutex));

     /* assign timing info from shared resource */
    filteredData.sec_elapsed = data_ptr->filteredData.sec_elapsed;
    filteredData.ms_elapsed = data_ptr->filteredData.ms_elapsed;
    filteredData.us_elapsed = data_ptr->filteredData.us_elapsed;
    for (j = 0; j < 4; j++)
    {
        /* assign filtered channel values from shared resource */
        filteredData.channels[j] = data_ptr->filteredData.channels[j]; 
    }
    /* release MUTEX on exiting critical section */
    pthread_mutex_unlock(&(data_ptr->mutex));
    /*******************************************
    *  CRITICAL SECTION ENDS
    ********************************************/

    /* detach the shared memory */
    if(munmap(data_ptr, sizeof(struct shared*)) == -1) {perror("munmap"); return;} 

    print_values(filteredData);
}
