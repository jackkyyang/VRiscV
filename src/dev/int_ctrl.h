

#ifndef __INT_CTRL_H__
    #define __INT_CTRL_H__

#include <stdint.h>
#include <pthread.h>

#include "../cpu/cpu_config.h"

// 初始化中断控制器
// 传入控制中断读写的互斥锁
// 返回二级中断的地址，最终将地址给各个设备
void int_init(pthread_mutex_t* screen_int_mutex, pthread_mutex_t* kbd_int_mutex,
              uint8_t* screen_int_ptr, uint8_t* kbd_int_ptr);

// 返回中断和中断值
// 内部完成中断仲裁和控制处理
// 返回值：
// 0：无中断
// > 0：存在中断，且返回中断的cause编号
MXLEN_T get_int_val();

void int_clr(MXLEN_T);

MXLEN_T get_int_id();

#endif