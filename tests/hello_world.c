
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[]){

    uint32_t a = 0xffff0000;
    int32_t b =  0xffff0000;
    uint64_t c = 0x18fff0000;
    uint64_t s0,s1,s3;
    if (argc > 2)
    {
        return 1;
    }
    s0 = a >> 5;
    s1 = b >> 5;
    s3 = ((uint32_t)(c)) >> 5;
    printf("%u logical right shift: %lu\n",a,s0);
    printf("%d arithmetic right shift: %ld\n",b,s1);
    printf("%lu arithmetic right shift: %ld\n",c,s3);


    return 0;
}