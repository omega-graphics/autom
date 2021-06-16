import os,sys,argparse,json
from enum import Enum


class ToolchainType(Enum):
    MSVC = 0
    LLVM = 1
    GCC = 2
    VULKAN = 3
    SPIRV = 4
    PERL = 5
    JAVA = 6

sub_regex = r"[\w|.]+"

gcc_regex = rf"gcc (?:\([\w| |.|-|_]+\)) ({sub_regex})"
clang_regex = rf"clang version ({sub_regex})"
lld_regex = rf"LLD ({sub_regex})"



def testForToolchain(type:ToolchainType):

    if type == ToolchainType.LLVM:
        os.popen("clang --version")


def determineToolchains() -> "list[dict[str]]":


    return []




def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--toolchain",choices=["msvc","llvm","gcc","vulkan","spirv","perl","java"],nargs="?")

    args = parser.parse_args()
    
    toolchain = determineToolchains()

    json.dump(toolchain,open("./AUTOM.toolchain"))
