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
        uint8_t* dev_exit;
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

    //键盘缓冲头数据结构
    // 软件在使用时，必须先lock住缓冲区，然后才可以读后面的数据
    // 在完成数据处理后，需要软件主动清空缓冲区内容
    typedef struct kbd_buf_h_t
    {
        uint32_t kbd_buf_lock; // 软件配置值，如果lock为1，则禁止设备向buf内写入数据
        uint32_t kbd_data_num; // 软件可读写，0, 代表没有有效数据
    }KeyBoardBufferH;

    //帧缓冲区头数据结构
    typedef struct frm_buf_h_t
    {
        // 软件配置值，如果lock为1，代表软件正在处理数据，不要让设备做内容搬运
        uint8_t  frm_buf_lock;
        uint8_t  frm_buf_change; // 0, 没有改变，不需要搬运帧缓冲区
        uint32_t frm_data_num; // 0, 代表没有有效数据
        uint16_t screen_width;
        uint16_t screen_height;
        uint32_t frame_buf_max_len;
    }FrameBufferH;

#endif //__DISPLAY_H__