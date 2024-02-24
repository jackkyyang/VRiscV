

#include "int_ctrl.h"
#include <pthread.h>
#include <stdint.h>


static pthread_mutex_t *screen_int_mutex;  // 屏幕中断锁
static pthread_mutex_t *kbd_int_mutex;     // 键盘中断锁

static uint8_t *screen_int_ptr;
static uint8_t *kbd_int_ptr;

static uint8_t effective_screen_int;
static uint8_t effective_kbd_int;

// TODO, 如何实现时钟中断和软件中断

void int_init(pthread_mutex_t* screen_int_mutex, pthread_mutex_t* kbd_int_mutex,
              uint8_t* screen_int_ptr, uint8_t* kbd_int_ptr)
{
    screen_int_mutex = screen_int_mutex;
    kbd_int_mutex = kbd_int_mutex;
    //
    screen_int_ptr = screen_int_ptr;
    kbd_int_ptr = kbd_int_ptr;
}

uint64_t get_int_val()
{
    // 参考特权指令集中对于mcause的定义
    // 3：M模式软件中断
    // 7：M模式时钟中断
    // 11： M模式外部中断
    // ≥16：平台定义的中断
    // 16：屏幕中断
    // 18：键盘中断

    // 中断值更新
    if(!pthread_mutex_trylock(screen_int_mutex)){
        //获得屏幕中断使用权, 更新屏幕中断有效值
        effective_screen_int = *screen_int_ptr;
        pthread_mutex_unlock(screen_int_mutex);
    }
    if(!pthread_mutex_trylock(kbd_int_mutex)){
        //获得屏幕中断使用权, 更新屏幕中断有效值
        effective_kbd_int = *kbd_int_ptr;
        pthread_mutex_unlock(kbd_int_mutex);
    }

    // 中断仲裁
    if (effective_screen_int)
        return 16;
    else if (effective_kbd_int)
        return 18;
    else
        return 0;
}
