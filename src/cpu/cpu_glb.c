#include "cpu_glb.h"

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

#include "cpu_glb.h"
#include <stdint.h>

static ExeStatus    cpu_exe_status;
static FetchStatus  cpu_fet_status;
static CPUMode cpu_mode;

ExeStatus *get_exe_st_ptr()
{
    return &cpu_exe_status;
}

ExeStatus *read_exe_st()
{
    return &cpu_exe_status;
}

FetchStatus *get_fet_st_ptr()
{
    return &cpu_fet_status;
}

CPUMode get_cpu_mode()
{
    return cpu_mode;
}

void set_cpu_mode(CPUMode next_mode)
{
    cpu_mode = next_mode;
}
