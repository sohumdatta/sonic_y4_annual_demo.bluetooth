#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <matlab/mex.h>
/* Shared memory implementation */
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "collect_data.h"

int get_sample(long* sec_elapsed, int* ms_elapsed, int* us_elapsed, double* filtered_channel, double* raw_channel)
{
    int i, j;

    /* initialize values */
    *sec_elapsed = 0;
    *ms_elapsed = 0;
    *us_elapsed = 0;
    for(i=0; i < 4; i++) filtered_channel[i] = 0.0;

    struct filtered_data filteredData;
    struct shared* data_ptr = (struct shared*) NULL;

    int shmid;
    
    /* create a shared memory (CANNOT CREATE)*/
    if((shmid = shm_open(SHARED_RESOURCE, O_RDWR , 0600)) == -1)
    {perror("shm_open"); return -1;}
    ftruncate(shmid, sizeof(struct shared));

    /* attach to the shared memory (CAN READ ONLY)*/
    data_ptr = (struct shared *) mmap(0, sizeof(struct shared),
                PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
    if(data_ptr == (struct shared*) (-1)) {perror("mmap"); return -2;}

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
        filteredData.raw_channels[j] = data_ptr->filteredData.raw_channels[j]; 
    }
    /* release MUTEX on exiting critical section */
    pthread_mutex_unlock(&(data_ptr->mutex));
    /*******************************************
    *  CRITICAL SECTION ENDS
    ********************************************/

    /* detach the shared memory */
    if(munmap(data_ptr, sizeof(struct shared*)) == -1) {perror("munmap"); return -3;} 


    /* assign values */
    *sec_elapsed = filteredData.sec_elapsed;
    *ms_elapsed = filteredData.ms_elapsed;
    *us_elapsed = filteredData.us_elapsed;

    for(i=0; i < 4; i++) filtered_channel[i] = filteredData.channels[i];
    for(i=0; i < 4; i++) raw_channel[i] = filteredData.raw_channels[i];

    return 0;
}

/* The gateway routine. */
void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[] )
{
    int i, return_val;
    size_t rows, cols;
   
    struct filtered_data filteredData;
 
    long* sec_elapsed;
    int* ms_elapsed;
    int* us_elapsed;
    double  * filtered_channel;
    double  * raw_channel;

    /* check for the proper number of arguments */
	/* sec_elapsed, ms_elapsed, us_elapsed, filtered_channel[4], raw_channel[4] */
    if(nrhs > 0)
      mexErrMsgIdAndTxt( "MATLAB:get_sample:invalidNumInputs",
              "No inputs required.");
    if(nlhs != 5)
      mexErrMsgIdAndTxt( "MATLAB:get_sample:maxlhs",
              "4 output arguments required: sec, ms, usec, filtered_channel");
  
    rows = 1; cols = 4;
 
    /* create a new array to return channel values and set the output pointer to it */
    plhs[0] = mxCreateNumericMatrix((mwSize) rows, (mwSize) rows, mxINT64_CLASS, mxREAL);
    sec_elapsed = (long *) mxGetPr(plhs[0]);
    if(sec_elapsed == NULL) mexErrMsgIdAndTxt("MATLAB:get_sample:NULL_return_vector", "sec_elapsed Null vector sent");

    plhs[1] = mxCreateNumericMatrix((mwSize) rows, (mwSize) rows, mxINT32_CLASS, mxREAL);
    ms_elapsed = (int *) mxGetPr(plhs[1]);
    if(ms_elapsed == NULL) mexErrMsgIdAndTxt("MATLAB:get_sample:NULL_return_vector", "ms_elapsed Null vector sent");
     
    plhs[2] = mxCreateNumericMatrix((mwSize) rows, (mwSize) rows, mxINT32_CLASS, mxREAL);
    us_elapsed = (int *) mxGetPr(plhs[2]);
    if(us_elapsed == NULL) mexErrMsgIdAndTxt("MATLAB:get_sample:NULL_return_vector", "us_elapsed Null vector sent");
     
    plhs[3] = mxCreateDoubleMatrix( (mwSize)rows, (mwSize)cols, mxREAL);
    filtered_channel = mxGetPr(plhs[3]);
    if(filtered_channel == NULL) mexErrMsgIdAndTxt("MATLAB:get_sample:NULL_return_vector", "filtered_channel Null vector sent");

    plhs[4] = mxCreateDoubleMatrix( (mwSize)rows, (mwSize)cols, mxREAL);
    raw_channel = mxGetPr(plhs[4]);
    if(raw_channel == NULL) mexErrMsgIdAndTxt("MATLAB:get_sample:NULL_return_vector", "raw_channel Null vector sent");

    /* call the C subroutine */
    /* call the C subroutine */
    return_val = get_sample(sec_elapsed, ms_elapsed, us_elapsed, filtered_channel, raw_channel);
    if(return_val < 0) mexErrMsgIdAndTxt("MATLAB:get_sample:return_val", "get_sample_C_failed");

    /* assign the values */
/*    *sec_elapsed = (double) 1.0 * filteredData.sec_elapsed;
    *ms_elapsed = (double) 1.0 * filteredData.ms_elapsed;
    *us_elapsed = (double) 1.0 * filteredData.us_elapsed;
    for(i=0; i < 4; i++) filtered_channel[i] = filteredData.channels[i];
*/
    return;
}
