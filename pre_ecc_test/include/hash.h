//
// Created by victor on 4/5/17.
//

#ifndef PRE_ECC_TEST_HASH_H
#define PRE_ECC_TEST_HASH_H



#include <iostream>
#include <algorithm>
#include <set>
#include <string>

#include "bch.h"
#include "tpdef.h"
//#include "md5.h"
//#include "sha256.h"

#define READ_LENGTH 4096
#define CONFIG_M 8
#define CONFIG_T 13

class hash{


public:

    uint8_t md5_file_comp(char *filename, char *dir_name);
    uint8_t sha256_file_comp(char *filename, char *dir_name);
    uint8_t sha1_file_comp(char *filename, char *dir_name);
    uint8_t ecc_file_comp(char *filename, char *dev_name, char *dir_name);
    uint8_t string_convert_uint64(uint8_t *content, uint64_t *num); //default length of content is 4

    hash();
    ~hash();

public:
    uint64_t chunk_num;
    double time_total;
    double time_aver;

private:
    std::set<std::string> list;
    void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen);
    struct bch_control *bch;
    uint64_t chunk_not_dup;

};

#endif //PRE_ECC_TEST_HASH_H