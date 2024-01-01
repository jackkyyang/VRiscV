/**
 * 来自于南京大学操作系统课 Demo
 * 课程主页：https://jyywiki.cn/OS/2022/index.html
 * 文件地址：https://jyywiki.cn/pages/OS/2022/demos/loader-static.c
 * 根据自己的需求，进行了少量修改
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
#include "../dev/mem_pool.h"

#define STK_SZ           (1 << 20)
#define ROUND(x, align)  (((uintptr_t)x) & ~(align - 1))
#define MOD(x, align)    (((uintptr_t)x) & (align - 1))
#define push(sp, T, ...) ({ *((T*)sp) = (T)__VA_ARGS__; \
                            sp = (void *)((uintptr_t)(sp) + sizeof(T)); })

void execve_(const char *file, char *argv[], char *envp[]) {
  // WARNING: This execve_ does not free process resources.
  int fd = open(file, O_RDONLY);
  assert(fd > 0);
  Elf32_Ehdr *h = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
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

  // 根据 Program header table file offset，拿到program header table的地址
  // 其中(char *)是为因为offset的单位是字节
  Elf32_Phdr *pht = (Elf32_Phdr *)((char *)h + h->e_phoff);
  for (int i = 0; i < h->e_phnum; i++) {
    // 取第i个propgram header
    Elf32_Phdr *p = &pht[i];
    if (p->p_type == PT_LOAD) {
      // int prot = 0;
      // // 设置权限，p_flags里保存了段的权限信息
      // // if (p->p_flags & PF_R) prot |= PROT_READ;
      // // if (p->p_flags & PF_W) prot |= PROT_WRITE;
      // // if (p->p_flags & PF_X) prot |= PROT_EXEC;
      // // 将段映射到本进程
      // void *ret = mmap(
      //   (void*)ROUND(p->p_vaddr, p->p_align),              // addr, 映射起始地址必须对齐到指定位置
      //   p->p_memsz + MOD(p->p_vaddr, p->p_align),   // length, 如果程序的起始位置没有对齐,在页表内部需要加一个偏移
      //   prot,                                       // protection
      //   MAP_PRIVATE | MAP_FIXED,                    // flags, private & strict
      //   fd,                                         // file descriptor
      //   (uintptr_t)ROUND(p->p_offset, p->p_align)); // offset，相对于文件头的偏移量
      // assert(ret != (void *)-1);

      // 将ELF中的内容搬运到内存池中
      assert(p->p_align <= ENTRY_SIZE); // 程序的对齐要求必须小于内存池的最小尺寸
      Elf32_Word mapped_size = 0; // 已经map的数据量
      Elf32_Word proc_size = 0;// 某次处理的数据量
      Elf32_Word remain_size = 0; // 未处理的数据量
      // 需要处理的数据总量，包括有效的数据和需要对齐的数据
      Elf32_Word total_size = p->p_filesz + MOD(p->p_vaddr,p->p_align);
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
          proc_size = (Elf32_Word)(ENTRY_SIZE - (uint64_t)ROUND(start_va, ENTRY_SIZE));
        }

        // 计算目的内存页的首地址，结果对齐到了内存池中每页的最小尺寸
        mem_pool_addr = mem_pool_lkup((uint64_t)ROUND(start_va, ENTRY_SIZE));
        // 计算目的地址，内存池基地址加上起始地址的offset
        mem_start_addr = (void*)(mem_pool_addr + MOD(start_va,ENTRY_SIZE));
        mem_end_addr = (void *)(mem_start_addr + proc_size);
        // 复制程序内容到虚拟机内存
        memcpy(mem_start_addr,src_align_addr,(size_t)proc_size);

        mapped_size +=proc_size;
      }

      memset(mem_end_addr, 0, p->p_memsz - p->p_filesz);
    }
  }
  close(fd);

  // static char stack[STK_SZ], rnd[16];
  // void *sp = (void *)ROUND(stack + sizeof(stack) - 4096, 16);
  // void *sp_exec = sp;
  // int argc = 0;

// TODO, 传入argc和argv，需要先建立栈空间
// 跳到程序入口


}

// int main(int argc, char *argv[], char *envp[]) {
//   if (argc < 2) {
//     fprintf(stderr, "Usage: %s file [args...]\n", argv[0]);
//     exit(1);
//   }
//   execve_(argv[1], argv + 1, envp);
// }