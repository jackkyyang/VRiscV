
#include "../src/utils/binary_reader.h"
#include <stdint.h>

int main(int argc, char *argv[]){

    const size_t buf_len = 100;
    uint32_t buf [buf_len];
    uint8_t b0,b1,b2,b3;
    int64_t rd_num = bin_file_rd32(buf,buf_len,argv[1]);
    size_t i = 0;

    // printf("%ld\n",rd_num);

    while (rd_num > 0)
    {
        b0 = (buf[i] & 0x000000ff);
        b1 = ((buf[i] & 0x0000ff00) >> 8);
        b2 = ((buf[i] & 0x00ff0000) >> 16);
        b3 = ((buf[i] & 0xff000000) >> 24);
        printf("%02x %02x %02x %02x\n",b0,b1,b2,b3);
        // printf("%08x\n",buf[i]);
        i ++;
        rd_num --;
    }

    return 0;


}