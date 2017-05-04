#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "hash.h"
#include "com_t.h"

int main(int argc, char **argv) {
    DIR *dir;
    struct dirent *ptr;
    char dir_name[256];
    char filename[256];
//the second string represent director name, and the third string represent the device name.
    if(argc != 3){
        std::cout<<"error:parameter is not completed!"<<std::endl;
        exit(0);
    }
    
    strcpy(dir_name, argv[1]);
    if((dir = opendir(dir_name)) == NULL){
        std::cout<<"error:open director error!"<<std::endl;
        exit(0);
    }

    //this module is to test sha256
    {
        hash sha256_h;


        std::cout<<"**************************************************"
                 <<"sha256 test start!"
                 <<"**************************************************"<<std::endl;
        std::cout<<std::left<<std::setw(30)<<"Filename"
                 <<std::left<<std::setw(30)<<"Total time(ms)"
                 <<std::left<<std::setw(30)<<"Chunk number"
                 <<std::left<<std::setw(30)<<"Average time(ms)"<<std::endl;
        while(NULL != (ptr = readdir(dir)) ){
            strcpy(filename, ptr->d_name);
            if(strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
                continue;
//            std::cout<<"We will start to test:"<<filename<<std::endl;
            sha256_h.sha256_file_comp(filename);

        }
    }

    closedir(dir);
    if((dir = opendir(dir_name)) == NULL){
        std::cout<<"error:open director error!"<<std::endl;
        exit(0);
    }

    //this module is to test sha1
    {
        hash sha1_h;


        std::cout<<"**************************************************"
                 <<"sha1 test start!"
                 <<"**************************************************"<<std::endl;
        std::cout<<std::left<<std::setw(30)<<"Filename"
                 <<std::left<<std::setw(30)<<"Total time(ms)"
                 <<std::left<<std::setw(30)<<"Chunk number"
                 <<std::left<<std::setw(30)<<"Average time(ms)"<<std::endl;
        while(NULL != (ptr = readdir(dir)) ){
            strcpy(filename, ptr->d_name);
            if(strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
                continue;
//            std::cout<<"We will start to test:"<<filename<<std::endl;
            sha1_h.sha1_file_comp(filename);

        }
    }

    closedir(dir);
    if((dir = opendir(dir_name)) == NULL){
        std::cout<<"error:open director error!"<<std::endl;
        exit(0);
    }

//this module is to test ecc
    {
        hash ecc;

        std::cout<<"**************************************************"
                 <<"ECC test start!"
                 <<"**************************************************"<<std::endl;
        std::cout<<std::left<<std::setw(30)<<"Filename"
                 <<std::left<<std::setw(30)<<"Total time(ms)"
                 <<std::left<<std::setw(30)<<"Chunk number"
                 <<std::left<<std::setw(30)<<"Average time(ms)"<<std::endl;
        while(NULL != (ptr = readdir(dir)) ){
            strcpy(filename, ptr->d_name);
            if(strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
                continue;
//            std::cout<<"We will start to test:"<<filename<<std::endl;
            ecc.ecc_file_comp(filename, argv[2]);

        }
    }
    closedir(dir);
    if((dir = opendir(dir_name)) == NULL){
        std::cout<<"error:open director error!"<<std::endl;
        exit(0);
    }
//this module is to test md5
    {
        hash md5_h;


        std::cout<<"**************************************************"
                 <<"MD5 test start!"
                 <<"**************************************************"<<std::endl;
        std::cout<<std::left<<std::setw(30)<<"Filename"
                 <<std::left<<std::setw(30)<<"Total time(ms)"
                 <<std::left<<std::setw(30)<<"Chunk number"
                 <<std::left<<std::setw(30)<<"Average time(ms)"<<std::endl;
        while(NULL != (ptr = readdir(dir)) ){
            strcpy(filename, ptr->d_name);
            if(strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
                continue;
//            std::cout<<"We will start to test:"<<filename<<std::endl;
            md5_h.md5_file_comp(filename);

        }
    }



    closedir(dir);
    return 0;
}