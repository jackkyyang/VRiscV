#! /usr/bin/python3

import argparse
import os
import re

def find_test_files(test_dir:str,test_set:str) -> list[str]:
    test_files = []
    if not os.path.exists(test_dir):
        print(f"Cannot find the Test case files in dir: {test_dir}")
    else:
        filenames= os.listdir(test_dir)
        # match_pattern = f"{test_set}-\w+"
        match_pattern = f"^(?!.*dump)({test_set}-\w+)" # 包括{test_set}并排除有dump的文件
        for file in filenames:
            if re.match(match_pattern,file):
                test_files.append(file)
    return test_files


def take_test(dut_path:str,test_file:str) -> bool:
    if not os.path.exists(dut_path):
        print("Cannot find executable file of DUT!")
    else:
        os.system(f"{dut_path} -s {test_file}")

    result_path = "./self_test_result.log"

    if not os.path.exists(result_path):
        print("Cannot find result of Test!")
        return True

    with open(result_path,"r") as f:
        if f.readline() != "1":
            print("Test fail! please check")
            return True
        else:
            return False





if __name__ == "__main__":

    test_set = [
            "rv32ui-p",
            "rv32um_p",
            "rv32mi_p",
            "rv32ua_p"
            ]

    parser = argparse.ArgumentParser("Take RV tests")
    parser.add_argument("-p","--test_path",help="the dir path of the tests file",required=True)
    parser.add_argument("-t","--test_set",help="the isa you want to test",choices=test_set,required=True)
    parser.add_argument("-d","--dut_path",help="file path of the dut",required=True)

    args = parser.parse_args()

    test_files = find_test_files(args.test_path,args.test_set)

    pass_num = 0

    for test in test_files:
        test_file = os.path.join(args.test_path,test)
        if take_test(args.dut_path,os.path.join(args.test_path,test)):
            break
        pass_num +=1

    print("*************************************")
    print(f"Total Test Num: {len(test_files)}")
    print(f"Pass Test Num: {pass_num}")
    print("*************************************")