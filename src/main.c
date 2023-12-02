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

#include "cpu/cpu.h"
#include "dev/memory.h"

void print_localtime(){
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("Simulation Start Time: %s", asctime(t));
    printf("--------------------------------\n");
}

uint64_t arguments_parse(int argc, char* argv[]){
    char* endptr;
    uint64_t num;
    if (argc == 2){
        num = strtoul(argv[1],&endptr,0);
        if (*endptr != '\0'){
            printf("Bad Argument Content: %s",argv[1]);
            return 0;
        } else if(num == 0){
            printf("Instruction Number must be positive!");
            return 0;
        }
        else {
            printf("--------------------------------\n");
            printf("Input a valid Argument: %lu\n",num);\
            return num;
        }
    } else {
        printf("Bad Arguments Number: %d\n",argc);
        return 0;
    }
}

int main(int argc, char* argv[]){

    clock_t begin, end;
    double time_cost;
    double ips = 0;
    uint64_t TIME_OUT_NUM = arguments_parse(argc,argv);
    uint64_t INST_NUM = 0;

    if (TIME_OUT_NUM == 0){
        return 0;
    }

    memory_init(DRAM128MB);

    print_localtime();
    begin = clock();
    INST_NUM = cpu_start(TIME_OUT_NUM);
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
