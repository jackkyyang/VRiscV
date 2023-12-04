
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../src/dev/memory.h"

#define BUF_DEPTH  64

static int test_fail;

void random_test(){

    uint8_t wr_buf [BUF_DEPTH];
    uint8_t rd_buf [BUF_DEPTH];

    uint64_t test_addr = (uint64_t)(rand());
    printf("Test Addr: %016lx \n",test_addr);

    for (int i = 0; i < BUF_DEPTH; i++)
    {
        wr_buf[i] = (uint8_t)(rand());
        printf("Write Data [%d]: %02x\n",i,wr_buf[i]);
    }

    int write_num = write_data(test_addr,BUF_DEPTH,CPU_FE,wr_buf);
    assert(write_num > 0);

    int read_num =read_data(test_addr,BUF_DEPTH,CPU_FE,rd_buf);
    assert(read_num > 0);

    for (int i = 0; i < BUF_DEPTH; i++)
    {
        if (wr_buf[i] != rd_buf[i]) {
            printf("Data compare Fail! addr[%lu + %d]: wr_data:%02x | rd_data:%02x \n",test_addr,i,wr_buf[i],rd_buf[i]);
            test_fail = 1;
        }
    }

}


int main(){

    memory_init(DRAM128MB);
    srand(1234);

    test_fail = 0;

    random_test();

    if (test_fail)
        printf("TEST FAILED!\n");
    else
        printf("TEST PASSED!\n");

    memory_free();
    return 0;
}
