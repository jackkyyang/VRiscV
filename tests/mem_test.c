
#include <stdint.h>
#include <stdio.h>
#include "../src/dev/memory.h"


int main(){

    memory_init(DRAM128MB);

    uint8_t wr_buf [8] = {0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f};
    uint8_t rd_buf [8];

    int write_num = write_data(0,8,CPU_FE,wr_buf);
    int read_num =read_data(0,8,CPU_FE,rd_buf);

    for (int i = 0; i < 8; i++)
    {
        printf("read data from addr[%d]: %02x",i,rd_buf[i]);
    }


    memory_free();
    return 0;
}
