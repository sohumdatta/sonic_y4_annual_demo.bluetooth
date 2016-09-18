%create_mex.m: create the mex file
clc;
mex -output get_sample get_sample_mex.c
