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
#include <getopt.h>

#include "cpu/cpu.h"
#include "dev/memory.h"
#include "dev/display.h"
#include "utils/simple_loader.h"
#include "utils/str_tools.h"
#include "include/comm.h"
#include "include/config.h"


#define RESET_ADDR 0x80000000
#define TIMEOUT 100000

void print_localtime(){
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("Simulation Start Time: %s", asctime(t));
    printf("--------------------------------\n");
}

static uint64_t timeout_num = TIMEOUT; // 默认值为TIMEOUT

static uint8_t self_test = 0;
static char* self_test_file;

static uint8_t bootloader = 0;
static char* bootloader_file;

static uint8_t set_entry_point = 0;
static uint64_t entry_addr = RESET_ADDR; // 默认值为RESET_ADDR

static uint8_t tracepc = 0;
static char* tracepc_logfile;

static uint8_t non_func = 0;

int arguments_parse(int argc, char* argv[]){

    int index;
    int iarg=0;
    int optflags;
    char *endptr;
    // 短参数
    // -t: timeout 的缩写，必须给出值
    // -s  self-test的缩写，必须给出文件路径
    char* short_opts = "t:s:";
    // 长参数
    // bootloader： bootloader的二进制文件
    // resetpc： reset时的PC，注意如果指定了-s选项，则不应该给出reset_addr
    // tracepc： trace开关，打开时需要给出trace log文件的路径
    // help：帮助
    const struct option longopts[] =
    {
      {"bootloader",    required_argument,      &optflags,  1},
      {"resetpc",       required_argument,      &optflags,  2},
      {"tracepc",       required_argument,      &optflags,  3},
      {"help",          no_argument,            0,          'h'},
      {"version",       no_argument,            0,          'v'},
      {0,0,0,0},
    };

    //turn off getopt error message
    // opterr=1;

    while(1)
    {
        endptr = NULL;
        iarg = getopt_long(argc, argv, short_opts, longopts, &index);
        if (iarg == -1)
            break;

        switch (iarg)
        {
          case 'h': // Help
            printf("Usage: VRiscV OPTIONS...\n");
            printf("    -t              TIMEOUT         integer, timeout number of instruction for the Virtual Machine\n");
            printf("    -s              filepath        enable self-test mode and set the filepath of the test executable file\n");
            printf("    --bootloader    filepath        filepath of the bootloader program\n");
            printf("    --resetpc       RESET_ADDR      integer, set the entry point of the Reset Vector\n");
            printf("    --tracepc       logfile         enable the function of PC tracing and Set the Log Filepath\n");
            printf("    --version                       display the version information.\n");
            printf("    --help                          display this help and exit\n");
            printf("\n<github: https://github.com/jackkyyang/VRiscV>\n");
            non_func = 1;
            break;

          case 'v': // Version
            printf("VRiscV  %s\n",PROJECT_VERSION);
            non_func = 1;
            break;

          case 's': // Self-Test
            self_test = 1;
            self_test_file = str_copy(optarg);
            printf("--------------\nEnter Self-test mode.\nTest case: %s\n--------------\n",self_test_file);
            break;

          case 't': // TIMEOUT
            timeout_num = strtoul(optarg,&endptr,0);
            if (*endptr != '\0'){
                printf("Bad Timeout Option Content: %s",optarg);
            }
            break;

          case 0: // 长参数
            if (optflags == 1) // 设定了bootloader
            {
                bootloader = 1;
                bootloader_file = str_copy(optarg);
            }
            else if (optflags == 2) // 设定了reset_pc
            {
                set_entry_point = 1;
                entry_addr = strtoul(optarg,&endptr,0);
            }
            else if (optflags == 3) // 开启了trace pc功能
            {
                tracepc = 1;
                tracepc_logfile = str_copy(optarg);
            }

            break;

          default:
            printf("Warning! getopt returned character code: [%s]\n", (char*)(&iarg));
        }
    }
    // 打印不能被解析的参数
    if (optind < argc) {
        printf("non-option elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

    // 检查是否有参数冲突
    if (self_test == 1 && set_entry_point == 1)
        printf("Warning! Set the entry point in Self-test mode. The Entry Point setted by user will be covered by address found in self-test file\n");

    if (self_test == 1 && bootloader == 1)
        printf("Warning! The Bootloader setted by user will be covered by address found in self-test file\n");


}


int main(int argc, char* argv[]){

    clock_t begin, end;
    double time_cost;
    double ips = 0;
    uint64_t INST_NUM = 0;
    FILE* tpc_fd = NULL;
    arguments_parse(argc,argv);

    memory_init(DRAM128MB);

    if (self_test){
        // 必须在初始化memory之后才能加载可执行文件
        entry_addr = simple_loader(self_test_file);
        // 清理历史log
        FILE* st_fd = fopen("./self_test_result.log","w");
        fprintf(st_fd,"0");
        fclose(st_fd);
    }

    // ----------------------------------
    // 检查参数
    // ----------------------------------
    if (non_func) // 不执行具体的功能，只显示信息
        return 0;

    if (timeout_num == 0){
        return 0;
    }
    if (entry_addr == ERR_ADDR) {
        return 0;
    }
    if (self_test== 0 && bootloader == 0){
        printf("Error! Must set the bootloader or self-test file!\n");
        return 0;
    }

    if (tracepc)
    {
        tpc_fd = fopen(tracepc_logfile,"w");
        if (tpc_fd == NULL)
        {
            printf("Error! Cannot open PC trace log: %s\n",tracepc_logfile);
            return 0;
        }
    }

    // ------------------------------------------
    // 启动CPU
    // ------------------------------------------
    print_localtime();
    begin = clock();
    INST_NUM = cpu_run(timeout_num,entry_addr,self_test,tpc_fd);
    end = clock();
    // ------------------------------------------
    // CPU 任务结束
    // ------------------------------------------

    time_cost = (double)(end-begin)/CLOCKS_PER_SEC;
    ips = (double)(INST_NUM)/time_cost;

    // 释放申请的内存
    memory_free(); // 对应 memory_init()
    if (self_test)
        free((void*)self_test_file);
    if (bootloader)
        free((void*)bootloader_file);
    if (tracepc) {
        free((void*)tracepc_logfile);
        fclose(tpc_fd);
    }

    printf("--------------------------------\n");
    printf("%lu Instructions Simulated!\n",INST_NUM);
    printf("Time Cost: %f\n",time_cost);
    printf("Instruction Number Per Second: %f\n",ips);
    printf("--------------------------------\n");

    return 0;
}
