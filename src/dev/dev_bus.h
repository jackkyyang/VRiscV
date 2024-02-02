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

// 定义了设备访问的操作
// 每个设备都有一个地址空间
// 每个地址有各自的访问权限，可能是RO，RW
// 对于设备操作，必须先得到锁之后才操作
#ifndef __DEV_BUS_H__
    #define __DEV_BUS_H__

#include <pthread.h>
#include <stdint.h>
#include "dev_config.h"

typedef struct dev_bus_param_t
{
    pthread_mutex_t *screen_mutex;
    pthread_mutex_t *kbd_mutex;
    void* screen_base;
    void* kbd_base;
} DevBusParam;


// 初始化用于设备地址空间的互斥锁
void dev_bus_init(DevBusParam param);

// 屏幕读操作，要求地址一定是4byte对齐
// addr: 读地址，无符号数，位宽为64 bits
// byte_num： 读取的（byte）数量
// data_buf: 数据buf，由master指定
// Return：0 成功，other：失败和错误码
int read_screen(uint64_t addr, uint8_t byte_num, uint8_t* data_buf);

// 屏幕写操作，地址一定是4byte对齐
// addr: 读地址，无符号数，位宽为64 bits
// byte_num： 读取的（byte）数量
// data_buf: 数据buf，由master指定
// Return：0 成功，other：失败和错误码
int write_screen(uint64_t addr, uint8_t byte_num, uint8_t* data_buf);


// 键盘读操作，要求地址一定是4byte对齐
// addr: 读地址，无符号数，位宽为64 bits
// byte_num： 读取的（byte）数量
// data_buf: 数据buf，由master指定
// Return：0 成功，other：失败和错误码
int read_kbd(uint64_t addr, uint8_t byte_num, uint8_t* data_buf);

// 键盘写操作，地址一定是4byte对齐
// addr: 读地址，无符号数，位宽为64 bits
// byte_num： 读取的（byte）数量
// data_buf: 数据buf，由master指定
// Return：0 成功，other：失败和错误码
int write_kbd(uint64_t addr, uint8_t byte_num, uint8_t* data_buf);

#endif // __DEV_BUS_H__