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

#ifndef __DISPLAY_H__
    #define __DISPLAY_H__

    #include <stdint.h>
    #include <pthread.h>

    typedef struct screen_init_param_t {
        // 共享地址
        pthread_mutex_t* screen_mem_mutex;  // 屏幕地址空间锁
        pthread_mutex_t* kbd_mem_mutex;     // 键盘地址空间锁
        void* screen_base;
        void* kbd_base;
        // 中断相关
        pthread_mutex_t* screen_int_mutex;  // 屏幕中断锁
        pthread_mutex_t* kbd_int_mutex;     // 键盘中断锁
        uint8_t* screen_int_ptr;
        uint8_t* kbd_int_ptr;
    } ScreenInitParam;

    // 启动一个屏幕显示线程
    void* screen_init(void*);

    // 关闭屏幕并退出
    void screen_close();

    // 向屏幕输出字符串
    void put_chars(const char* output_data,int len);

    // 获得屏幕宽度
    int get_screen_width();

    // 获得屏幕高度
    int get_screen_height();


#endif //__DISPLAY_H__