# ----------------------------------------------------------------------------------------
# MIT License
#
# Copyright (c) 2023 jackkyyang
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# ------------------------------------------------------------------------------------------


from ast import literal_eval
from copy import copy
from pprint import pprint
from tomlkit.toml_file import TOMLFile
from os import path
from typing import Union
from re import search

def toml_parse(file_path:str) -> dict[str,Union[str,dict]]:
    to_doc = TOMLFile(file_path).read()
    return literal_eval(str(to_doc))


class Field():
    def __init__(self,name:str,bit_pos:list[str],offset:int=0) -> None:
        self._name = name
        self._bit_pos = bit_pos
        self._offset = offset
        self.__len_calc()
        self.__mask_gen()

    def __len_calc(self):
        self._len = 0
        for pos in self._bit_pos:
            search_result = search("(\d+):(\d+)|(\d+)",pos)
            if search_result is not None:
                if (search_result.groups()[2] is None):
                    self._len += abs(int(search_result.groups()[0]) -
                                     int(search_result.groups()[1]) + 1)
                else:
                    self._len += 1
            else:
                raise Exception("Format Error! Cannot parse the information of the field")

    def __len__(self):
        return self._len

    def __mask_gen(self):
        offset = 0
        self._mask_list:list[tuple[int,str]] = []
        for pos in self._bit_pos:
            search_result = search("(\d+):(\d+)|(\d+)",pos)
            if search_result is not None:
                tmp_mask = ["0"] * 32
                if (search_result.groups()[2] is None):
                    end = max(int(search_result.groups()[0]),
                              int(search_result.groups()[1]))
                    start = min(int(search_result.groups()[0]),
                                int(search_result.groups()[1]))
                    len = end - start + 1
                    for i in range(start,end + 1):
                        tmp_mask[i] = "1"
                else:
                    bit_pos = int(search_result.groups()[2])
                    tmp_mask[bit_pos] = "1"
                    len = 1
                tmp_mask.reverse()
                self._mask_list.append((offset,"".join(tmp_mask),))
                offset += len
            else:
                raise Exception("Format Error! Cannot parse the information of the field")

    def _get_c_code(self,op:str,op_len=32):
        tmp_list = []
        for msk in self._mask_list:
            shift_offset = "".join(reversed(msk[1])).index("1")  - msk[0] - self._offset
            if shift_offset >= 0:
                tmp_list.append(f"(({op} & 0b{msk[1]}) >> {shift_offset})")
            else:
                tmp_list.append(f"(({op} & 0b{msk[1]}) << {abs(shift_offset)})")
        express = " |\n           ".join(tmp_list)
        return f"    {self._name}\n        = {express};"

    def __repr__(self) -> str:
        tmp_list = [f"[offset: {t[0]} - mask: {t[1]}]" for t in self._mask_list]
        return "\n".join(tmp_list)

# instruction uop
class Op():
    def __init__(self,name:str,encoding:dict,out_fields:list[str],input_fields:dict[str,Field]) -> None:
        self.name = name
        self.fix_fields = list[encoding.keys()]
        self.determine_code =  f"is_{self.name} = {self.dec_code(encoding)};"
        self.dispatch_args = out_fields


    def dec_code(self,encoding:dict,field_mask:list[str] = None):
        tmp_list = []
        for key in encoding:
            if field_mask is not None and key in field_mask:
                continue
            else:
                tmp_list.append(f"({key} == {encoding[key]})")
        return " && ".join(tmp_list)

    def dispatch_code(self):
        tmp_list = [f"{field} = {field}" for field in self.dispatch_args]
        args_express = ",".join(tmp_list)
        return f"{self.name}({args_express});"

class DecTree():
    def __init__(self,field_name:str,match_table:dict,fields:list[str]) -> None:
        self.field_name = field_name
        if "match" in match_table:
            if type(match_table["match"]) is str:
                match_express = match_table["match"]
                self.cond = f"{self.field_name} == {match_express}"
                for key in match_table:
                    if key in fields:
                        self.sub_nodes = [DecTree(key,match_table[key],fields)]

class DecTreeRoot(DecTree):
    pass

# decoder
class Decoder():
    def __init__(self,table_path:str,ext_list:list[str]) -> None:
        if not path.isfile(table_path):
            raise Exception(f"Error! the argument: {table_path} is not a file path!")

        self.__dec_table = toml_parse(table_path)
        self.__get_all_extentions(ext_list)
        self.__get_arch_info()
        self.__glb_field_table = {}
        self.__flags :dict[str:list[Op]] = {}
        if "field" in self.__dec_table:
            for key in self.__dec_table["field"]:
                self.__glb_field_table[key] = Field(key,self.__dec_table["field"][key])

    def __get_arch_info(self):
        self.__arch_name :str = self.__dec_table["ISA"]
        self.__xlen :str = self.__dec_table["XLEN"]

    @property
    def xlen(self):
        return self.__xlen

    @property
    def arch_name(self):
        return self.__arch_name

    def __get_all_extentions(self,ext_list):
        tmp_list = []
        for ext in ext_list:
            if ext in self.__dec_table:
                tmp_list.append(ext)

        self.__ext_in_table = tuple(tmp_list)

    @property
    def ext_in_table(self):
        return self.__ext_in_table

    def _get_const(self,const_name:str):
        if const_name not in self.__dec_table["CONST"]:
            raise Exception(f"Cannot find Constant: {const_name} in decode table")
        return self.__dec_table["CONST"][const_name]


    def dec_table_parse(self,ext_name:str):
        self.dec_tree = []
        for ext in self.ext_in_table:
            group_dict = self.__dec_table[ext]





    # -------------------------- print info -----------------------------------
    def print_table(self) -> None:
        pprint(self.__dec_table,indent=4)

    def print_arch_info(self):
        print("--------- Arch Information ---------")
        print(f"- Arch: {self.__arch_name}")
        print(f"- Extensions: {self.ext_in_table}")
        print("------------------------------------")

    def print_fields(self):
        print("global field table:")
        pprint(self.__glb_field_table,indent=4)


if __name__ == "__main__":
    decoder = Decoder("./misc/rv32-dec.toml",["I"])
    decoder.print_table()
    decoder.print_arch_info()
    decoder.print_fields()

    print(Field("shift_imm",["31:25"])._get_c_code("inst"))
    print(Field("shamt",["24:20"])._get_c_code("inst"))
    print(Field("branch_imm",["11:8","30:25","7","31"],1)._get_c_code("inst"))
    print(Field("jal_imm",["30:21","20","19:12","31"],1)._get_c_code("inst"))
    print(Field("str_imm",["11:7","31:25"])._get_c_code("inst"))
    print(Field("li_imm",["31:12"],12)._get_c_code("inst"))
    print(Field("succ",["23:20"])._get_c_code("inst"))
    print(Field("pred",["27:24"])._get_c_code("inst"))
    print(Field("fm",["31:28"])._get_c_code("inst"))
