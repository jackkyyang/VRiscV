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

#include "cpu/cpu.h"

void print_localtime(){
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("--------------------------------\n");
    printf("Local Time: %s", asctime(t));
    printf("--------------------------------\n");
}


int main(int argc, char* argv[]){

    clock_t begin, end;
    double time_cost;
    uint64_t INST_NUM = 100;
    double ips = 0;
    print_localtime();
    begin = clock();
    cpu_start(INST_NUM);
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
