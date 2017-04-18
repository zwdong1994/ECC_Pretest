//
// Created by victor on 4/5/17.
//

#ifndef PRE_ECC_TEST_HASH_H
#define PRE_ECC_TEST_HASH_H



#include <iostream>

#include "tpdef.h"
#include "md5.h"
#include "sha256.h"

#define READ_LENGTH 4096

class hash{


public:

    uint8_t md5_file_comp(char *filename);
    uint8_t sha256_file_comp(char *filename);
    uint8_t ecc_file_comp(char *filename, char *dev_name);
    uint8_t string_convert_uint64(uint8_t *content, uint64_t *num); //default length of content is 4

    hash();
    ~hash();

public:
    uint64_t chunk_num;
    double time_total;
    double time_aver;

};

#endif //PRE_ECC_TEST_HASH_H