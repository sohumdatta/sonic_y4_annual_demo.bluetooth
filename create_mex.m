%create_mex.m: create the mex file
mex -output get_sample get_sample_mex.c -lrt -lpthread
