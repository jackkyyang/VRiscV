/*
MIT License

Copyright (c) 2024 jackkyyang

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

#include <stdint.h>
#include <pthread.h>
#include <stdio.h>

#include "dev_bus.h"
#include "int_ctrl.h"
#include "dev_config.h"

static pthread_mutex_t* s_screen_mutex;
static pthread_mutex_t* s_kbd_mutex;

// 映射到设备地址空间的内存
static void* screen_base;
static void* kbd_base;

void dev_bus_init(DevBusParam param){
    s_screen_mutex = param.screen_mutex;
    s_kbd_mutex = param.kbd_mutex;
    screen_base = param.screen_base;
    kbd_base = param.kbd_base;
}

static inline int read_dev(uint64_t offset, uint8_t byte_num, uint8_t *data_buf, pthread_mutex_t* dev_mutex, void* dev_base) {

    // acquire the lock of device memory
    if (pthread_mutex_lock(dev_mutex) != 0){
        printf("Meet MUTEX lock error during reading devices!");
        return 1;
    }

    for (uint8_t i = 0; i < byte_num; i++)
    {
        data_buf[i] = ((uint8_t*)dev_base)[offset+i];
    }

    pthread_mutex_unlock(dev_mutex);
    return 0;
}

static inline int write_dev(uint64_t offset, uint8_t byte_num, uint8_t *data_buf, pthread_mutex_t* dev_mutex, void* dev_base) {
    uint8_t* wr_ptr = (uint8_t*)dev_base;

    // acquire the lock of device memory
    if (pthread_mutex_lock(dev_mutex) != 0){
        printf("Meet MUTEX lock error during writing devices!");
        return 2;
    }

    for (uint8_t i = 0; i < byte_num; i++) {
        wr_ptr[i] = data_buf[i];
    }

    pthread_mutex_unlock(dev_mutex);
    return 0;
}

int read_screen(uint64_t addr, uint8_t byte_num, uint8_t *data_buf)
{
    return read_dev((addr - SCR_BASE),byte_num,data_buf,s_screen_mutex,screen_base);
}

int write_screen(uint64_t addr, uint8_t byte_num, uint8_t *data_buf)
{
    return write_dev((addr - SCR_BASE),byte_num,data_buf,s_screen_mutex,screen_base);
}

int read_kbd(uint64_t addr, uint8_t byte_num, uint8_t *data_buf)
{
    return read_dev((addr - KBD_BASE),byte_num,data_buf,s_kbd_mutex,kbd_base);
}

int write_kbd(uint64_t addr, uint8_t byte_num, uint8_t *data_buf)
{
    return write_dev((addr - KBD_BASE),byte_num,data_buf,s_kbd_mutex,kbd_base);
}

// 实现简单的中断控制器的读操作
//
int read_int(uint64_t addr, uint8_t byte_num, uint8_t *data_buf)
{
    uint64_t offset = addr - INTCTRL_BASE;
    if (byte_num != 4)
    {
        printf("Error! Must read 4 byte from int_ctrl!");
        return 1;
    }

    if (offset == 0)
    {
        *((MXLEN_T*)data_buf) = get_int_id();
        return  0;
    }
    else {
        return 1;
    }

}

int write_int(uint64_t addr, uint8_t byte_num, uint8_t *data_buf)
{
    uint64_t offset = addr - INTCTRL_BASE;

    if (byte_num != 4)
    {
        printf("Error! Must write 4 byte from int_ctrl!");
        return 1;
    }

    if (offset == 4)
    {
        MXLEN_T int_id = *((MXLEN_T*)data_buf);
        int_clr(int_id);
        return  0;
    }
    else {
        return 1;
    }
}