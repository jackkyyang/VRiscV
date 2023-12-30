/*
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
*/


#include "front_end.h"
#include "cpu_config.h"
#include "../dev/memory.h"

void instruction_fetch(FetchParam* fetch_param, uint32_t* inst_buf)
{
    FetchStatus *f_st_ptr = get_fet_st_ptr();
    int inst_fetch = read_data(fetch_param->pc,FETCH_NUM*4,CPU_FE,(uint8_t*)inst_buf);
    if (inst_fetch == 0)
    {
        f_st_ptr->err_id = get_ifu_fault();
        f_st_ptr->inst_num = 0;
    }
    else {
        f_st_ptr->inst_num = FETCH_NUM;
    }
}
