import os, sys, shutil
import io

is_win = sys.platform == "Windows"
is_unix = sys.platform == "Linux" or sys.platform == "macOS"


configs = {
    "llvm-unix": {
        "cc": "clang",
        "cxx": "clang++",
        "ld": "ld.lld",
        "ar": "ar",
    },
    "llvm-win": {
        "cc": "clang-cl",
        "cxx": "clang-cl",
        "ld": "lld-link",
        "ar": "llvm-lib",
    },
    "gcc": {
        "cc": "gcc",
        "cxx": "g++",
        "ld": "link",
        "ar": "lib",
    },
    "msvc": {
        "cc": "cl",
        "cxx": "cl",
        "ld": "link",
        "ar": "lib",
    }
}


def findProgram(cmd: str):
    return shutil.which(cmd)


def main():
    if is_win:
        has_vs = findProgram("vswhere")
        if has_vs is None:
            raise RuntimeError("Visual Studio Not Installed..")

        stream = os.popen("vswhere -latest -property installationPath")
        vs_loc = stream.read()
        print(vs_loc)

    elif is_unix:
        has_make = findProgram("make")


main()

