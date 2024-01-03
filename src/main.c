/*
MIT License

Copyright (c) 2023 jackkyyang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "cpu/cpu.h"
#include "dev/memory.h"
#include "utils/simple_loader.h"


#define RESET_ADDR 0x80000000
#define TIMEOUT 100000

void print_localtime(){
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("Simulation Start Time: %s", asctime(t));
    printf("--------------------------------\n");
}

static uint64_t timeout_num = TIMEOUT; // 默认值为TIMEOUT
static char* load_file;
static uint8_t load_file_valid = 0;

void arguments_parse(int argc, char* argv[]){
    char* endptr;
    load_file_valid = 0;
    if (argc == 2){
        timeout_num = strtoul(argv[1],&endptr,0);
        if (*endptr != '\0'){
            printf("Bad Argument Content: %s",argv[1]);
            return;
        } else if(timeout_num == 0){
            printf("Instruction Number must be positive!");
            return;
        }
        else {
            printf("--------------------------------\n");
            printf("Input a valid Argument: %lu\n",timeout_num);\
            return;
        }
    }
    else if (argc == 3)
    {
        if (strcmp(argv[1],"-s"))
        {
            printf("Must use option:-l, but found a %s\n",argv[1]);
            return;
        }
        printf("--------------\nEnter Self-test mode.\nTest case: %s\n--------------\n",argv[2]);
        load_file_valid = 1;
        load_file = argv[2];
        return;
    }


    else {
        printf("Bad Arguments Number: %d\n",argc);
        return;
    }
}

int main(int argc, char* argv[]){

    clock_t begin, end;
    double time_cost;
    double ips = 0;
    uint64_t INST_NUM = 0;
    uint64_t entry_addr = RESET_ADDR; // 默认值为RESET_ADDR

    arguments_parse(argc,argv);

    if (timeout_num == 0){
        return 0;
    }

    memory_init(DRAM128MB);

    if (load_file_valid)
    {
        // 必须在初始化memory之后才能加载可执行文件
        entry_addr = simple_loader(load_file);
    }

    print_localtime();
    begin = clock();
    INST_NUM = cpu_run(timeout_num,entry_addr);

    end = clock();
    time_cost = (double)(end-begin)/CLOCKS_PER_SEC;
    ips = (double)(INST_NUM)/time_cost;
    printf("--------------------------------\n");
    printf("%lu Instructions Simulated!\n",INST_NUM);
    printf("Time Cost: %f\n",time_cost);
    printf("Instruction Number Per Second: %f\n",ips);
    printf("--------------------------------\n");

    return 0;
}
