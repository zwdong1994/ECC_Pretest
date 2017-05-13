//
// Created by victor on 4/5/17.
//
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include "sha256.h"

double time_total = 0.0;
long chunk_num = 0;

//get time
double get_time(void)
{
    struct	timeval	mytime;
    gettimeofday(&mytime,NULL);
    return (mytime.tv_sec*1.0+mytime.tv_usec/1000000.0);
}

//compute every file's sha256 time
int file_read(char *file_name)
{

}

//compute average time
int compute_time(double time, )
{

}