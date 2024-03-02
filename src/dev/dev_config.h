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

#ifndef __DEV_CONFIG_H__
    #define __DEV_CONFIG_H__

    #include <stdint.h>

#define MEM1KB    (1<<10)
#define MEM1MB    (1<<20)
#define MEM4KB    (MEM1KB * 4)
#define MEM8KB    (MEM1KB * 8)
#define MEM16KB   (MEM1KB * 16)
#define MEM64KB   (MEM1KB * 64)
#define MEM256KB  (MEM1KB * 256)
#define MEM512KB  (MEM1KB * 512)
#define MEM128MB  (MEM1MB * 128)

#define DRAM_SIZE MEM128MB
#define SCR_SIZE  MEM16KB
#define KBD_SIZE  MEM4KB
#define ROM_SIZE  MEM8KB
#define INTCTRL_SIZE  MEM4KB

// Memory Map
// 主存
#define DRAM_BASE 0x80000000
#define DRAM_END  (DRAM_BASE + DRAM_SIZE - 1)

// 显示设备
// 显示器地址空间为256KB
#define SCR_BASE  0x00040000 // 256KB
#define SCR_END  (SCR_BASE + SCR_SIZE - 1)

// 键盘
// 键盘地址空间为4KB
#define KBD_BASE  0x00020000 //128KB
#define KBD_END  (KBD_BASE + KBD_SIZE - 1)

// ROM
// 容量设置为 8KB
#define ROM_BASE  0x00000000
#define ROM_END  (KBD_BASE + ROM_SIZE - 1)

// INTCTRL
// 容量设置为 4KB
#define INTCTRL_BASE  0x00010000 // 64K
#define INTCTRL_END  (INTCTRL_BASE + INTCTRL_SIZE - 1)

#endif // __DEV_CONFIG_H__