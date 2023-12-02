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

#define STK_SZ           (1 << 20)
#define ROUND(x, align)  (void *)(((uintptr_t)x) & ~(align - 1))
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
  assert(h->e_ident[0]  = 0x7f &&
         h->e_ident[1]  = 0x45 &&
         h->e_ident[2]  = 0x4c &&
         h->e_ident[3]  = 0x46 &&
         h->e_ident[4]  = 0x01 &&
         h->e_ident[5]  = 0x01 &&
         h->e_ident[6]  = 0x01 &&
         h->e_ident[7]  = 0x00 &&
         h->e_ident[8]  = 0x00 &&
         h->e_ident[9]  = 0x00 &&
         h->e_ident[10] = 0x00 &&
         h->e_ident[11] = 0x00 &&
         h->e_ident[12] = 0x00 &&
         h->e_ident[13] = 0x00 &&
         h->e_ident[14] = 0x00 &&
         h->e_ident[15] = 0x00);
  assert(h->e_type == ET_EXEC && h->e_machine == EM_RISCV);

  // 根据 Program header table file offset，拿到program header table的地址
  // 其中(char *)是为因为offset的单位是字节
  Elf32_Phdr *pht = (Elf32_Phdr *)((char *)h + h->e_phoff);
  for (int i = 0; i < h->e_phnum; i++) {
    // 取第i个propgram header
    Elf32_Phdr *p = &pht[i];
    if (p->p_type == PT_LOAD) {
      int prot = 0;
      // 设置权限，p_flags里保存了段的权限信息
      if (p->p_flags & PF_R) prot |= PROT_READ;
      if (p->p_flags & PF_W) prot |= PROT_WRITE;
      if (p->p_flags & PF_X) prot |= PROT_EXEC;
      // 将段映射到本进程
      void *ret = mmap(
        ROUND(p->p_vaddr, p->p_align),              // addr, rounded to ALIGN
        p->p_memsz + MOD(p->p_vaddr, p->p_align),   // length
        prot,                                       // protection
        MAP_PRIVATE | MAP_FIXED,                    // flags, private & strict
        fd,                                         // file descriptor
        (uintptr_t)ROUND(p->p_offset, p->p_align)); // offset
      assert(ret != (void *)-1);
      memset((void *)(p->p_vaddr + p->p_filesz), 0, p->p_memsz - p->p_filesz);
    }
  }
  close(fd);

  static char stack[STK_SZ], rnd[16];
  void *sp = ROUND(stack + sizeof(stack) - 4096, 16);
  void *sp_exec = sp;
  int argc = 0;

  // argc
  while (argv[argc]) argc++;
  push(sp, intptr_t, argc);
  // argv[], NULL-terminate
  for (int i = 0; i <= argc; i++)
    push(sp, intptr_t, argv[i]);
  // envp[], NULL-terminate
  for (; *envp; envp++) {
    if (!strchr(*envp, '_')) // remove some verbose ones
      push(sp, intptr_t, *envp);
  }
  // auxv[], AT_NULL-terminate
  push(sp, intptr_t, 0);
  push(sp, Elf32_auxv_t, { .a_type = AT_RANDOM, .a_un.a_val = (uintptr_t)rnd } );
  push(sp, Elf32_auxv_t, { .a_type = AT_NULL } );

  // TODO, 替换成RISCV指令
  // asm volatile(
  //   "mov $0, %%rdx;" // required by ABI
  //   "mov %0, %%rsp;"
  //   "jmp *%1" : : "a"(sp_exec), "b"(h->e_entry));
}

// int main(int argc, char *argv[], char *envp[]) {
//   if (argc < 2) {
//     fprintf(stderr, "Usage: %s file [args...]\n", argv[0]);
//     exit(1);
//   }
//   execve_(argv[1], argv + 1, envp);
// }