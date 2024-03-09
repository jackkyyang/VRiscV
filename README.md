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

## 编译和构建

### 开发平台

本项目的开发和测试平台为:

`WSL2 + Ubuntu 20.04.6 LTS`

在其它Linux环境仍有可能正常工作，但暂未进行验证。

### 安装依赖

```
sudo apt-get update
sudo apt-get install cmake pkg-config libgtk-3-dev
```

### 编译构建

本项目使用CMake构建

```
cmake -B build
cmake --build build
```

## 使用虚拟机

#### 执行自测程序

```
./build/VRiscV -s ./tests/isa_testcase/rv32ui-p/rv32ui-p-add
```

#### 其它功能

```
./build/VRiscV --help
```

## 开发计划

### ~~阶段1(已完成)~~

完成RISC-V RV32IM 指令的模拟

完成访存系统的模拟

成果：基本的IM指令自测完成

支持指令扩展：

    **RV32 I，M，Zicsr，Zifencei**

### 阶段2

1. ~~支持中断相关的处理(已完成)~~
2. ~~完成部分设备的模拟（串口，显示）(已完成)~~
3. 时钟
4. 软件中断

成果：完成基本的系统调用，能够完成hello world 打印

### 阶段3

完成剩余设备的模拟

1. 支持指令扩展**A、F**
2. 文件系统

成果：能够完成简单的自启动

### 阶段4

移植bootloader和内核

成果：能启动操作系统
