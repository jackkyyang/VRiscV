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
#include <signal.h>
#include <pthread.h>

#include "cpu/cpu.h"
#include "dev/memory.h"
#include "dev/display.h"
#include "dev/dev_config.h"
#include "dev/int_ctrl.h"
#include "dev/dev_bus.h"
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
static FILE* tpc_fd = NULL;

static uint8_t non_func = 0;

// 线程控制
static uint8_t cpu_exit;
static uint8_t dev_exit;

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
            printf("    -t              TIMEOUT         integer, timeout number of instruction for self-test mode\n");
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


// 注册互斥锁
static pthread_mutex_t screen_mem_mutex;  // 屏幕地址空间锁
static pthread_mutex_t kbd_mem_mutex;     // 键盘地址空间锁
static pthread_mutex_t screen_int_mutex;  // 屏幕中断锁
static pthread_mutex_t kbd_int_mutex;     // 键盘中断锁
static uint8_t screen_int;
static uint8_t kbd_int;
// 注意，由于共享内存地址和中断指针值要在中断控制器初始化完成后才确定
// 因此参数需要在main中确定

static void* screen_mem_base;
static void* keyboard_mem_base;

static ScreenInitParam screen_param;
static DevBusParam dev_bus_param;

static clock_t begin, end;

static CPUParam cpu_params;

// 子线程TID
static pthread_t cpu_tid;
static pthread_t display_tid;

static int cpu_started = 0;
static int dev_started = 0;

static void prinf_runtime_info(){
    double time_cost = (double)(end-begin)/CLOCKS_PER_SEC;
    uint64_t inst_num = get_iid();
    double ips = (double)(inst_num)/time_cost;
    printf("--------------------------------\n");
    printf("%lu Instructions Simulated!\n",inst_num);
    printf("Time Cost: %f\n",time_cost);
    printf("Instruction Number Per Second: %f\n",ips);
    printf("--------------------------------\n");
}

// 资源释放
static void resource_free(){
    memory_free(); // 对应 memory_init()
    if (self_test)
        free((void*)self_test_file);
    if (bootloader)
        free((void*)bootloader_file);
    if (tracepc) {
        free((void*)tracepc_logfile);
        fclose(tpc_fd);
    }
    // 在子线程结束后回收共享的内存
    if (self_test == 0)
    {
        free(screen_mem_base);
        free(keyboard_mem_base);
        // 销毁所有互斥锁
        pthread_mutex_destroy(&screen_mem_mutex);
        pthread_mutex_destroy(&kbd_mem_mutex);
    }
    // 回收中断锁
    pthread_mutex_destroy(&screen_int_mutex);
    pthread_mutex_destroy(&kbd_int_mutex);

}

void exit_handler(){
    // 因为是signal是异步的,无法精确获得cpu线程的结束时间
    end = clock();
    printf("\nGet SIGINT, Exit Virtual Machine!\n");
    cpu_exit = 1;
    dev_exit = 1;
    prinf_runtime_info();
    resource_free();
    exit(0);
}

int main(int argc, char* argv[]){
    arguments_parse(argc,argv);

    // ----------------------------------
    // 检查参数
    // ----------------------------------
    if (non_func) // 不执行具体的功能，只显示信息
        return 0;

    if (timeout_num == 0){
        return 0;
    }

    if (self_test== 0 && bootloader == 0){
        printf("Error! Must set the bootloader or self-test file!\n");
        return 0;
    }


    print_localtime();
    // 自测时也要初始化中断控制器，因此需要初始化中断锁
    pthread_mutex_init(&screen_int_mutex,NULL);
    pthread_mutex_init(&kbd_int_mutex,NULL);
    // 注册中断
    int_init(&screen_int_mutex,&kbd_int_mutex,&screen_int,&kbd_int);

    // 自测时不需要启动显示和键盘设备
    if (self_test ==0) {

        //------------------------------------
        // 准备初始化设备所需要的资源，包括内存和锁
        //------------------------------------
        // 初始化互斥锁
        pthread_mutex_init(&screen_mem_mutex,NULL);
        pthread_mutex_init(&kbd_mem_mutex,NULL);

        // 申请设备空间的存储
        // 对于显示设备的地址，需要实现dual buffer
        // 一份buffer在设备和CPU之间共享，另一份私有
        // 每隔一段时间，由显示设备调用memcpy
        screen_mem_base = malloc((size_t)SCR_SIZE); // 默认screen的空间为16KB
        keyboard_mem_base = malloc((size_t)KBD_SIZE); // 默认screen的空间为4KB

        if (screen_mem_base == NULL) {
            printf("Malloc Error for screen memory!");
            return 0;
        }
        if (keyboard_mem_base == NULL) {
            printf("Malloc Error for keyboard memory!");
            return 0;
        }

        // 设置总线参数
        dev_bus_param.kbd_base = keyboard_mem_base;
        dev_bus_param.kbd_mutex = &kbd_mem_mutex;
        dev_bus_param.screen_base = screen_mem_base;
        dev_bus_param.screen_mutex = &screen_mem_mutex;
        // 初始化总线
        dev_bus_init(dev_bus_param);

        //------------------------------------
        // 开启设备进程
        //------------------------------------
        // 准备参数
        screen_param.screen_int_mutex = &screen_int_mutex;
        screen_param.screen_int_ptr = &screen_int;
        screen_param.screen_mem_mutex = &screen_mem_mutex;
        screen_param.screen_base = screen_mem_base;

        screen_param.kbd_int_mutex = &kbd_int_mutex;
        screen_param.kbd_int_ptr = &kbd_int;
        screen_param.kbd_mem_mutex = &kbd_mem_mutex;
        screen_param.kbd_base = keyboard_mem_base;
        screen_param.dev_exit = &dev_exit;

        int t_creat_result = pthread_create(&display_tid,NULL,screen_init,&screen_param);
        if (t_creat_result)
        {
            //线程创建错误
            printf("Found Error during pthread_creat, code is [%d]\n",t_creat_result);
            return 0;
        }
        dev_started =1 ;
        printf("Start Display thread, TID is [%lX]\n",display_tid);
    }

    // ------------------------------------------
    // 启动CPU
    // ------------------------------------------
    // 初始化配置环境
    int init_err_flag = 0;

    // 建立trace PC 文件
    if (tracepc)
    {
        tpc_fd = fopen(tracepc_logfile,"w");
        if (tpc_fd == NULL)
        {
            printf("Error! Cannot open PC trace log: %s\n",tracepc_logfile);
            init_err_flag = 1;
        }
    }

    // 初始化主存，必须在load 自测文件之前
    memory_init();
    if (self_test){
        // 必须在初始化memory之后才能加载可执行文件
        entry_addr = simple_loader(self_test_file);
        // 清理历史log
        FILE* st_fd = fopen("./self_test_result.log","w");
        fprintf(st_fd,"0");
        fclose(st_fd);
    }
    if (bootloader)
    {
        entry_addr = simple_loader(bootloader_file);
    }


    if (entry_addr == ERR_ADDR) {
        init_err_flag = 2;
    }

    // ----------------------------
    // 进入CPU
    // ----------------------------
    if (init_err_flag == 0)
    {
        // 没有初始化错误
        signal(SIGINT, exit_handler); // 注册中断函数，用户可自行停止虚拟机进程
        // 参数准备
        cpu_params.TIME_OUT = timeout_num;
        cpu_params.entry_addr = entry_addr;
        cpu_params.self_test = self_test;
        cpu_params.tpc_fd = tpc_fd;
        cpu_params.cpu_exit = &cpu_exit;
        cpu_params.start_time = &begin;
        cpu_params.end_time = &end;
        // 启动CPU线程
        int cpu_res = pthread_create(&cpu_tid,NULL,cpu_run,&cpu_params);
        if (cpu_res)
        {
            //线程创建错误
            printf("Found Error during creat CPU thread, code is [%d]\n",cpu_res);
        }
        cpu_started = 1;
        printf("Start CPU thread, TID is [%lX]\n",cpu_tid);
        // 虚拟机自行退出
    }
    else {
        printf("--------------------------------\n");
        printf("Init Fail! Fail Code:[%d] \n",init_err_flag);
        printf("--------------------------------\n");
    }

    //---------------------------------
    // 资源回收
    //---------------------------------
    // 等待线程结束
    char* retval = NULL;

    if (dev_started == 1) {
        pthread_join(display_tid,(void**)&retval);

        // 如果屏幕线程关闭, 立刻关闭CPU线程
        if (cpu_started == 1) {
            cpu_exit = 1;
        }
    }

    if (cpu_started == 1)
        pthread_join(cpu_tid,(void**)&retval);

    // 释放申请的内存
    resource_free();
    prinf_runtime_info();
    return 0;
}
