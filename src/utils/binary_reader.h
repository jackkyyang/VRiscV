// ----------------------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2023 jackkyyang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ------------------------------------------------------------------------------------------

#ifndef __BINARY_READER_H__
    #define __BINARY_READER_H__

#include <stdint.h>
#include <stdio.h>

// 功能：打开并一次性读取指定二进制文件，将所有二进制数据都放入指定地址
// buf：存储数据的地址，每个地址32b
// buf_len：    buf最大的深度，不能超过该深度
// file_path： 文件路径
// 返回值：     读取数据的长度，单位为32b
int64_t bin_file_rd32(uint32_t *buf, size_t buf_len, char *file_path);



#endif //__BINARY_READER_H__