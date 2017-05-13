cmake_minimum_required(VERSION 3.5)
project(pre_ecc_test)

ADD_DEFINITIONS(-g -W -Wall -lrt -lcrypto -lpthread  -D_GLIBCXX_USE_CXX11_ABI=0 -fpermissive)
include_directories(./include /usr/include)
link_libraries("/usr/lib/x86_64-linux-gnu/librt.so")
link_libraries("/usr/lib/x86_64-linux-gnu/libcrypto.a")
link_directories(/lib/x86_64-linux-gnu)
SET(CMAKE_C_COMPILER "g++")
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp include/hash.h sourcefile/hash.cpp include/tpdef.h include/com_t.h sourcefile/com_t.cpp
        sourcefile/md5.c include/md5.h sourcefile/sha256.c include/sha256.h include/sha1.h sourcefile/sha1.c include/bch.h
        sourcefile/bch.c)
add_executable(pre_ecc_test ${SOURCE_FILES} )
