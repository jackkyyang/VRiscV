
#ifndef __CPU_CONFIG_H__
    #define __CPU_CONFIG_H__

#include <stdint.h>

typedef enum {
    I16 = 1,
    I32 = 2,
    I64 = 3
} InstSet;

typedef uint8_t FetchWidth;

#ifdef RV64
    #define RV_XLEN 64;
    #define INST_SET I64
#else
    #define RV_XLEN 32;
    #define INST_SET I32
#endif

#define RESET_ADDR 0x80000000
#define FETCH_NUM 1

#define CPU_TIME_OUT 10000


#endif //__CPU_CONFIG_H__

