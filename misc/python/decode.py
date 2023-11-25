from ast import literal_eval
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
        self._mask_list= []
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





class Decoder():
    def __init__(self,table_path:str,ext_list:list[str]) -> None:
        if not path.isfile(table_path):
            raise Exception(f"Error! the argument: {table_path} is not a file path!")

        self.__dec_table = toml_parse(table_path)
        self.__get_all_extentions()
        self.__get_arch_info()
        if "field" in self.__dec_table:
            self.__glb_field_table = self.__dec_table["field"]
        else:
            self.__glb_field_table = {}

    def __get_arch_info(self):
        self.__arch_name :str = self.__dec_table["ISA"]
        self.__xlen :str = self.__dec_table["XLEN"]

    @property
    def xlen(self):
        return self.__xlen

    @property
    def arch_name(self):
        return self.__arch_name

    def __get_all_extentions(self):
        self.__ext_in_table:tuple[str] = tuple(key for key in self.__dec_table["extensions"])

    @property
    def ext_in_table(self):
        return self.__ext_in_table

    def print_table(self) -> None:
        pprint(self.__dec_table,indent=4)

    def print_arch_info(self):
        print(f"Arch: {self.__arch_name}")
        print(f"Extensions: {self.ext_in_table}")

    def _get_const(self,const_name:str):
        if const_name not in self.__dec_table["CONST"]:
            raise Exception(f"Cannot find Constant: {const_name} in decode table")
        return self.__dec_table["CONST"][const_name]


if __name__ == "__main__":
    # decoder = Decoder("./misc/rv32_dec.toml",["I"])
    # decoder.print_table()
    # decoder.print_arch_info()

    rd = Field("rd",["11:7"],0)
    print(len(rd))
    print(rd._mask_list)
    imm = Field("rd",["11:8","30:25","7","31"],0)
    print(len(imm))
    print(imm._mask_list)
