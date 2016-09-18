#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
/* Shared memory implementation */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
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

    struct filtered_data* data_ptr = (struct filtered_data*) NULL;
    struct filtered_data filteredData;

    int shmid;
    size_t filteredData_size = sizeof(struct filtered_data);    
    
    /* Generate key for sharing filteredData */
    errno = 0;
    key_t key = ftok(OUTPUT_FILE, 'R');
    if(key == -1) {perror("ftok"); return;}
    
    /* create a shared memory (no create) */
    if((shmid = shmget(key, filteredData_size, 0644)) == -1)
    {perror("shmget"); return;}

    /* attach to the shared memory, (READ ONLY) */
    data_ptr = (struct filtered_data*) shmat(shmid, (void*) 0, SHM_RDONLY);
    if(data_ptr == (struct filtered_data*) (-1)) {perror("shmat"); return;}

    /* copy from shared memory so that another process can read it */
    memcpy(&filteredData, data_ptr, filteredData_size);
           
    /* detach from the shared memory segment */
    if(shmdt(data_ptr) == -1) {perror("shmdt"); return;} 
    
    print_values(filteredData);
}
