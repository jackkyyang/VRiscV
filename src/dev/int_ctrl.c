

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#include "int_ctrl.h"
#include "../cpu/sys_reg.h"

static pthread_mutex_t* s_screen_int_mutex;  // 屏幕中断锁
static pthread_mutex_t* s_kbd_int_mutex;     // 键盘中断锁

static uint8_t* s_screen_int_ptr;
static uint8_t* s_kbd_int_ptr;

static uint8_t effective_screen_int;
static uint8_t effective_kbd_int;

static const MXLEN_T SCREEN_INT_ID = 16;
static const MXLEN_T KBD_INT_ID = 18;

static MXLEN_T glb_int_id;

// TODO, 如何实现时钟中断和软件中断

void int_init(pthread_mutex_t* screen_int_mutex, pthread_mutex_t* kbd_int_mutex,
              uint8_t* screen_int_ptr, uint8_t* kbd_int_ptr)
{
    s_screen_int_mutex = screen_int_mutex;
    s_kbd_int_mutex = kbd_int_mutex;
    //
    s_screen_int_ptr = screen_int_ptr;
    s_kbd_int_ptr = kbd_int_ptr;
}

MXLEN_T get_int_val()
{
    // 参考特权指令集中对于mcause的定义
    // 3：M模式软件中断
    // 7：M模式时钟中断
    // 11： M模式外部中断
    // ≥16：平台定义的中断
    // 16：屏幕中断
    // 18：键盘中断

    // 中断值更新
    MIP int_pend = {0};

    if(!pthread_mutex_trylock(s_screen_int_mutex)){
        //获得屏幕中断使用权, 更新屏幕中断有效值
        effective_screen_int = *s_screen_int_ptr;
        int_pend.scr_int = effective_screen_int;
        pthread_mutex_unlock(s_screen_int_mutex);
    }
    if(!pthread_mutex_trylock(s_kbd_int_mutex)){
        //获得屏幕中断使用权, 更新屏幕中断有效值
        effective_kbd_int = *s_kbd_int_ptr;
        int_pend.kbd_int = effective_kbd_int;
        pthread_mutex_unlock(s_kbd_int_mutex);
    }

    set_mip(int_pend);

    // 中断仲裁
    if (effective_screen_int)
        glb_int_id = SCREEN_INT_ID;
    else if (effective_kbd_int)
        glb_int_id = KBD_INT_ID;
    else
        glb_int_id = 0;

    return glb_int_id;
}

void int_clr(MXLEN_T int_id)
{
    if (int_id == SCREEN_INT_ID)
    {
        if(pthread_mutex_lock(s_screen_int_mutex) == 0){
            //获得屏幕中断使用权, 更新屏幕中断有效值
            *s_screen_int_ptr = 0;
            pthread_mutex_unlock(s_screen_int_mutex);
        }
        else {
            printf("Meet Errors during clr screen interupt!");
        }
    }
    else if (int_id == KBD_INT_ID)
    {
        if(pthread_mutex_lock(s_kbd_int_mutex) == 0){
            //获得屏幕中断使用权, 更新屏幕中断有效值
            *s_kbd_int_ptr = 0;
            pthread_mutex_unlock(s_kbd_int_mutex);
        }
        else {
            printf("Meet Errors during clr keyboard interupt!");
        }
    }

}

MXLEN_T get_int_id(){
    return glb_int_id;
}