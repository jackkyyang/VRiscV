from ast import literal_eval
from pprint import pprint
from tomlkit.toml_file import TOMLFile

def toml_parse(file_path:str) -> dict:
    to_doc = TOMLFile(file_path).read()
    return literal_eval(str(to_doc))


if __name__ == "__main__":
    pprint(toml_parse("./misc/rv32_dec.toml"),indent=4)