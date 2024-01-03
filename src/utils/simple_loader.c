/**
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

描述：
 * 简单的程序加载器，为虚拟机加载静态连接的二进制自测程序
 * 参考了南京大学操作系统课的加载器Demo，根据自己的需求，进行了修改
 * 课程主页：https://jyywiki.cn/OS/2022/index.html
 * 文件地址：https://jyywiki.cn/pages/OS/2022/demos/loader-static.c
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "../dev/mem_pool.h"
#include "../include/comm.h"

#define STK_SZ           (1 << 20)


uint64_t simple_loader(const char *file) {

  //获取ELF文件大小
  struct stat statbuf;
  stat(file,&statbuf);
  long elf_filesize = statbuf.st_size;
  size_t elf_map_size = (size_t)ROUND(elf_filesize,4096);
  if (elf_map_size > 4096 * 1024)
  {
    printf("Warning! The size of elf file is larger than 4MB, please check! ---filepath:%s\n",file);
  }
  int fd = open(file, O_RDONLY);
  uint64_t entry_addr=0;
  assert(fd > 0);
  Elf32_Ehdr *h = mmap(NULL, elf_map_size, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(h != (void *)-1);
  // check the magic number
  assert(h->e_ident[0]  == 0x7f &&
         h->e_ident[1]  == 0x45 &&
         h->e_ident[2]  == 0x4c &&
         h->e_ident[3]  == 0x46 &&
         h->e_ident[4]  == 0x01 &&
         h->e_ident[5]  == 0x01 &&
         h->e_ident[6]  == 0x01 &&
         h->e_ident[7]  == 0x00 &&
         h->e_ident[8]  == 0x00 &&
         h->e_ident[9]  == 0x00 &&
         h->e_ident[10] == 0x00 &&
         h->e_ident[11] == 0x00 &&
         h->e_ident[12] == 0x00 &&
         h->e_ident[13] == 0x00 &&
         h->e_ident[14] == 0x00 &&
         h->e_ident[15] == 0x00);
  assert(h->e_type == ET_EXEC && h->e_machine == EM_RISCV);

  entry_addr= (uint64_t) h->e_entry;// 获取程序起始地址

  // 根据 Program header table file offset，拿到program header table的地址
  // 其中(char *)是为因为offset的单位是字节
  Elf32_Phdr *pht = (Elf32_Phdr *)((char *)h + h->e_phoff);
  for (int i = 0; i < h->e_phnum; i++) {
    // 取第i个propgram header
    Elf32_Phdr *p = &pht[i];
    if (p->p_type == PT_LOAD) {
      // 将ELF中的内容搬运到内存池中
      assert(p->p_align <= ENTRY_SIZE); // 程序的对齐要求必须小于内存池的最小尺寸
      uint64_t mapped_size = 0; // 已经map的数据量
      uint64_t proc_size = 0;// 某次处理的数据量
      uint64_t remain_size = 0; // 未处理的数据量
      // 需要处理的数据总量，包括有效的数据和需要对齐的数据
      uint64_t total_size = (uint64_t)(p->p_filesz + MOD(p->p_vaddr,p->p_align));
      void* start_va = 0;
      void* elf_start_addr = 0; // 定位到ELF文件中程序头表的起始偏移量
      void* src_align_addr = 0; // ELF中处理的程序偏移量
      uint8_t* mem_pool_addr; // 每次映射到虚拟机主存的页表
      void* mem_start_addr; // 每次映射到虚拟机主存的起始地址
      void* mem_end_addr; // 每次映射到虚拟机主存的结束地址
      elf_start_addr = (void*)((uint8_t*)h + p->p_offset);
      while (mapped_size < total_size)
      {

        // 定位到当前ELF中处理的程序偏移量
        src_align_addr = (void*)(ROUND(elf_start_addr,p->p_align) + mapped_size);
        // 当前映射的起始VA,对齐到要求的地址
        start_va = (void*)(ROUND(p->p_vaddr,p->p_align) + mapped_size);
        // 计算当前映射的数据量
        remain_size = total_size - mapped_size;
        if (remain_size <= ENTRY_SIZE)
        {
          proc_size = remain_size;
        }
        else {
          proc_size = (ENTRY_SIZE - (uint64_t)ROUND(start_va, ENTRY_SIZE));
        }

        // 计算目的内存页的首地址，结果对齐到了内存池中每页的最小尺寸
        uint64_t mem_pool_va_tag = (uint64_t)ROUND(start_va, ENTRY_SIZE);
        mem_pool_addr = mem_pool_lkup(mem_pool_va_tag);
        // 计算目的地址，内存池基地址加上起始地址的offset
        mem_start_addr = (void*)(mem_pool_addr + MOD(start_va,ENTRY_SIZE));
        mem_end_addr = (mem_start_addr + (proc_size));
        // 复制程序内容到虚拟机内存
        memcpy(mem_start_addr,src_align_addr,(size_t)proc_size);

        // for (int i = 0; i < proc_size; i+=4)
        // {
        //   printf("value at elf start addr: %x\n",*((uint32_t*)(src_align_addr + i)));
        // }

        mapped_size +=proc_size;
      }

      memset(mem_end_addr, 0, p->p_memsz - p->p_filesz);
    }
  }
  munmap((void*)h,elf_map_size);
  close(fd);

  // static char stack[STK_SZ], rnd[16];
  // void *sp = (void *)ROUND(stack + sizeof(stack) - 4096, 16);
  // void *sp_exec = sp;
  // int argc = 0;

  return entry_addr;

}
