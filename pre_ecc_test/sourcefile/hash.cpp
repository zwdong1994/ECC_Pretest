// Created by victor on 4/5/17.
//
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <aio.h>

#include "hash.h"
#include "com_t.h"

hash::hash() {
    chunk_num = 0;
    time_total = 0.0;
    time_aver = 0.0;
}

hash::~hash() {
    if(time_total > 0) {
        time_aver = time_total / chunk_num;
        std::cout <<std::endl<< "**************************"<<"This option's time performance***********************"<<std::endl;
        std::cout << "The total time is " << time_total <<"ms"<< std::endl;
        std::cout << "The chunk number is " << chunk_num <<"ms"<< std::endl;
        std::cout << "The average time is " << time_aver <<"ms"<< std::endl<<std::endl;
    }
}

uint8_t hash::ecc_file_comp(char *filename, char *dev_name) {
    FILE *fp;
    uint8_t chk_cont[4096];
    uint64_t another_chk = 0;
    struct aiocb64 myaio;
    struct aiocb64 *cblist[1];
    double stat_time = 0.0, fin_time = 0.0;
    cp_t time_cpt;
    char file_dir[256];

//    double xor1=0.0, xor2=0.0;

    int fd = open(dev_name, O_RDWR|O_LARGEFILE);
    if(fd==-1) //add by zwd
    {
        std::cout<<"open "<<dev_name<<" error!"<<std::endl;
        exit(0);
    }
    bzero( (char *)cblist, sizeof(cblist) );
    bzero((char *)&myaio,sizeof(struct aiocb64));
    myaio.aio_buf=malloc(4097);
    if(!myaio.aio_buf)
        perror("malloc");
    myaio.aio_fildes = fd;
    myaio.aio_nbytes = 4096;
    myaio.aio_offset = 0;
    aio_write64(&myaio);
    while (EINPROGRESS == aio_error64(&myaio )) ; //EINPROGRESS represent the request are not complete.

    sprintf(file_dir, "/mnt/cdrom/kernel/%s", filename);
    if((fp = fopen(file_dir, "r")) == NULL){
        std::cout<<"Open file error!The file name is: "<<filename<<std::endl;
        return 0;
    }
    while(1){
        memset(chk_cont, 0, READ_LENGTH);
        if(fgets((char *)chk_cont, READ_LENGTH, fp) == NULL ){
//            std::cout<<"The file: "<<filename<<" ecc computation is already finished"<<std::endl;
            break;
        }
        double elps_time = 0.0;

        stat_time = time_cpt.get_time();
//read overhead
//        xor1 = time_cpt.get_time();
        cblist[0] = &myaio;
        aio_read64(&myaio);
        aio_suspend64(cblist, 1, NULL);
//        xor2 = time_cpt.get_time();
//        std::cout<<"xor2 - xor1 = "<<(xor2-xor1)*1000<<std::endl;
//read over
//xor start

        for(int i=0; i < 512; i++) {
            uint64_t xor_num = 0;
            string_convert_uint64(chk_cont + i * 8, &xor_num);
            another_chk = another_chk ^ xor_num;
        }

//xor finish
        fin_time = time_cpt.get_time();
        elps_time = (fin_time - stat_time) * 1000;//ms
//        std::cout<<"finish - start = "<<elps_time<<std::endl;
        time_cpt.cp_all(elps_time);
    }
    time_total += time_cpt.time_total;
    chunk_num += time_cpt.chunk_num;
    time_cpt.cp_aver(filename);
    return 1;
}

uint8_t hash::md5_file_comp(char *filename) {
    FILE *fp;
    uint8_t chk_cont[4096];
    uint8_t hv[16];
    double stat_time = 0.0, fin_time = 0.0;
    cp_t time_cpt;
    char file_dir[256];

    sprintf(file_dir, "/mnt/cdrom/kernel/%s", filename);
    if((fp = fopen(file_dir, "r")) == NULL){
        std::cout<<"Open file error!The file name is:"<<filename<<std::endl;
        return 0;
    }
    while(1){
        memset(chk_cont, 0, READ_LENGTH);
        if(fgets((char *)chk_cont, READ_LENGTH, fp) == NULL ){
//            std::cout<<"The file:"<<filename<<" md5 computation is already finished"<<std::endl;
            break;
        }
        double elps_time = 0.0;

        stat_time = time_cpt.get_time();
//md5 func
        md5(chk_cont, 4096, hv);
//md5 finish
        fin_time = time_cpt.get_time();
        elps_time = (fin_time - stat_time) * 1000;//ms
        time_cpt.cp_all(elps_time);
    }
    time_cpt.cp_aver(filename);
    time_total += time_cpt.time_total;
    chunk_num += time_cpt.chunk_num;

    return 1;
}

uint8_t hash::sha256_file_comp(char *filename) {
    FILE *fp;
    uint8_t chk_cont[4096];
    sha256_context ctx;
    uint8_t hv[32];
    double stat_time = 0.0, fin_time = 0.0;
    cp_t time_cpt;
    char file_dir[256];

    sprintf(file_dir, "/mnt/cdrom/kernel/%s", filename);
    if((fp = fopen(file_dir, "r")) == NULL){
        std::cout<<"Open file error!The file name is:"<<file_dir<<std::endl;
        return 0;
    }
    while(1){
        memset(chk_cont, 0, READ_LENGTH);
        if(fgets((char *)chk_cont, READ_LENGTH, fp) == NULL ){
//            std::cout<<"The file:"<<filename<<" sha256 computation is already finished"<<std::endl;
            break;
        }
        double elps_time = 0.0;

        stat_time = time_cpt.get_time();
//sha256 func
        sha256_init(&ctx);
        sha256_hash(&ctx, chk_cont, 4096);
        sha256_done(&ctx, hv);
//sha256 finish
        fin_time = time_cpt.get_time();
        elps_time = (fin_time - stat_time) * 1000;//ms
        time_cpt.cp_all(elps_time);
    }
    time_cpt.cp_aver(filename);
    time_total += time_cpt.time_total;
    chunk_num += time_cpt.chunk_num;
    return 1;
}

uint8_t hash::string_convert_uint64(uint8_t *content, uint64_t *num) {
    uint64_t mid;
    mid = (uint64_t)content[0];
    *num = *num ^ ((mid<<56)>>56);
    mid = (uint64_t)content[1];
    *num = *num ^ ((mid<<56)>>48);
    mid = (uint64_t)content[2];
    *num = *num ^ ((mid<<56)>>40);
    mid = (uint64_t)content[3];
    *num = *num ^ ((mid<<56)>>32);
    mid = (uint64_t)content[4];
    *num = *num ^ ((mid<<56)>>24);
    mid = (uint64_t)content[5];
    *num = *num ^ ((mid<<56)>>16);
    mid = (uint64_t)content[6];
    *num = *num ^ ((mid<<56)>>8);
    mid = (uint64_t)content[7];
    *num = *num ^ (mid<<56);
    return 1;
}