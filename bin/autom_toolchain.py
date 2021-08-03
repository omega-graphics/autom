import io
import os,sys,argparse,json,shutil
from enum import Enum


class ToolchainType(Enum):
    MSVC = 0
    LLVM = 1
    GCC = 2
    VULKAN = 3
    SPIRV = 4
    PERL = 5
    JAVA = 6


def testCase(name:str):
    print(f"Checking for {name}")
    res = shutil.which(name)
    if res is not None:
        print(f"-- Checking for {name} - found")
    else:
        print(f"-- Checking for {name} - not found")
    return res is not None


Toolchain = "dict[str,bool]"


def testToolchainCase(name:str,progs:"list[str]") -> Toolchain:
    print(f"Testing for {name}")
    res:Toolchain = {}
    success = True
    for p in progs:
        r = testCase(p)
        res[p] = r
        if not r:
            success = False
    
    if success:
        print(f"Testing for {name} - succeeded")
    else:
        print(f"Testing for {name} - failed")
    return res


def determineToolchains() -> "dict[str,Toolchain]":

    res = {}
    res["LLVM"] = testToolchainCase("LLVM",["clang","lld","lldb"])

    res["GCC"] = testToolchainCase("GCC",["gcc","ld","ar","gdb"])

    res["MSVC"] = testToolchainCase("MSVC",["cl","lib","link"])

    return res


def main(__args):
    parser = argparse.ArgumentParser()
    parser.add_argument("--toolchain",choices=["msvc","llvm","gcc","vulkan","spirv","perl","java"],nargs="?")

    args = parser.parse_args(__args)
    
    toolchain = determineToolchains()

    json.dump(toolchain,io.open("./AUTOM.toolchain","w"))


if __name__ == "__main__":
    sys.argv.pop(0)
    main(sys.argv)