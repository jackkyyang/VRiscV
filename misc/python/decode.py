from ast import literal_eval
from pprint import pprint
from tomlkit.toml_file import TOMLFile
from os import path
from typing import Union

def toml_parse(file_path:str) -> dict[str,Union[str,dict]]:
    to_doc = TOMLFile(file_path).read()
    return literal_eval(str(to_doc))


class Field():
    def __init__(self,name:str,bit_pos:list[int],offset:int=0) -> None:
        self._len = len(bit_pos)
        self._name = name
        self._bit_pos = bit_pos
        self._offset = offset
        self._mask_gen()

    def __len__(self):
        return self._len

    def _mask_gen(self):
        # mask 是 mask_shift 组成的对
        prev_pos = self._bit_pos[0] - 1
        tmp_mask = ["0"] * 32
        offset = 0
        self._mask_list = []

        for i in range(self._len):
            if self._bit_pos[i] != prev_pos + 1:
                tmp_mask.reverse()
                self._mask_list.append((offset,"".join(tmp_mask),))
                offset = i
                tmp_mask = ["0"] * 32

            tmp_mask[self._bit_pos[i]] = "1"
            prev_pos = self._bit_pos[i]

            if i == self._len -1:
                tmp_mask.reverse()
                self._mask_list.append((offset,"".join(tmp_mask),))



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

    rd = Field("rd",[7, 8, 9, 10, 11],0)
    print(rd._mask_list)

    imm = Field("rd", [8,9,10,11,25,26,27,28,29,30,7,31],1)
    print(imm._mask_list)