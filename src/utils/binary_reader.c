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

#include <stdio.h>
#include <stdint.h>
#include "binary_reader.h"

# define PRINT_FILE_MSG(msg,file_path) printf("%s(line-%d): "#msg", Path: '%s'\n",__FILE__,__LINE__,(file_path))

int64_t bin_file_rd32(uint32_t *buf, size_t buf_len, char *file_path)
{
    size_t rd_num = 0;

    FILE *file = fopen(file_path,"rb");
    if (file == NULL)
    {
        PRINT_FILE_MSG(Cannot open binary file!,file_path);
        return -1;
    }

    rd_num = fread(buf,sizeof(uint32_t),buf_len,file);

    if (ferror(file))
    {
        PRINT_FILE_MSG(Error happened during reading file!,file_path);
        fclose(file);
        return -1;
    }
    if (fclose(file))
    {
        PRINT_FILE_MSG(Error happened during closing file!,file_path);
        return -1;
    }

    return rd_num;

}
