# VRsicV
[![Build](https://github.com/jackkyyang/VRiscV/actions/workflows/build.yml/badge.svg)](https://github.com/jackkyyang/VRiscV/actions/workflows/build.yml)

RSICV 体系结构虚拟机，用来仿真RSICV CPU和设备，基本目标是能用来启动一个真实的操作系统

计划模拟的部分：

1. CPU core
2. 主存
3. 显示设备
4. 串口
5. 硬盘
6. BootROM

## 开发计划

### ~~阶段1(已完成)~~

完成RISC-V RV32IM 指令的模拟

完成访存系统的模拟

成果：基本的IM指令自测完成

支持指令扩展：

    **RV32 I，M，Zicsr，Zifencei**

### 阶段2

1. 支持中断相关的处理
2. 完成部分设备的模拟（串口，显示）
3. 支持指令扩展**A**

成果：完成基本的系统调用，能够完成hello world 打印

### 阶段3

完成剩余设备的模拟

成果：能够完成简单的自启动

### 阶段4

移植bootloader和内核

成果：能启动操作系统
