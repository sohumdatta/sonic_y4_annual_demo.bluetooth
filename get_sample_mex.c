#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
/* Shared memory implementation */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <matlab/mex.h>
#include "collect_data.h"

int get_sample(long* sec_elapsed, int* ms_elapsed, int* us_elapsed, double* filtered_channel)
{
    int i, j;

    /* initialize values */
    *sec_elapsed = 0;
    *ms_elapsed = 0;
    *us_elapsed = 0;
    for(i=0; i < 4; i++) filtered_channel[i] = 0.0;

    struct filtered_data* data_ptr = (struct filtered_data*) NULL;
    struct filtered_data filteredData;

    int shmid;
    size_t filteredData_size = sizeof(struct filtered_data);    
    
    /* Generate key for sharing filteredData */
    errno = 0;
    key_t key = ftok(OUTPUT_FILE, 'R');
    if(key == -1) {perror("ftok"); return -1;}
    
    /* create a shared memory (no create) */
    if((shmid = shmget(key, filteredData_size, 0644)) == -1)
    {perror("shmget"); return -1;}

    /* attach to the shared memory, (READ ONLY) */
    data_ptr = (struct filtered_data*) shmat(shmid, (void*) 0, SHM_RDONLY);
    if(data_ptr == (struct filtered_data*) (-1)) {perror("shmat"); return -1;}

    /* copy from shared memory so that another process can read it */
    memcpy(&filteredData, data_ptr, filteredData_size);
           
    /* detach from the shared memory segment */
    if(shmdt(data_ptr) == -1) {perror("shmdt"); return -1;} 

    /* assign values */
    *sec_elapsed = filteredData.sec_elapsed;
    *ms_elapsed = filteredData.ms_elapsed;
    *us_elapsed = filteredData.us_elapsed;
    for(i=0; i < 4; i++) filtered_channel[i] = filteredData.channels[i];

    
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

    /* check for the proper number of arguments */
    if(nrhs > 0)
      mexErrMsgIdAndTxt( "MATLAB:get_sample:invalidNumInputs",
              "No inputs required.");
    if(nlhs != 4)
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

    /* call the C subroutine */
    return_val = get_sample(sec_elapsed, ms_elapsed, us_elapsed, filtered_channel);
    if(return_val < 0) mexErrMsgIdAndTxt("MATLAB:get_sample:return_val", "get_sample_C_failed");

    /* assign the values */
/*    *sec_elapsed = (double) 1.0 * filteredData.sec_elapsed;
    *ms_elapsed = (double) 1.0 * filteredData.ms_elapsed;
    *us_elapsed = (double) 1.0 * filteredData.us_elapsed;
    for(i=0; i < 4; i++) filtered_channel[i] = filteredData.channels[i];
*/
    return;
}
