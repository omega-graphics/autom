import io
import os, sys, argparse, json, shutil
from enum import Enum
from typing import Optional, Union

toolchainConfig: "list[dict[str,Union[str,dict[str]]]]"

class ToolchainType(Enum):
    MSVC = 0
    LLVM = 1
    GCC = 2
    VULKAN = 3
    SPIRV = 4
    PERL = 5
    JAVA = 6


def testCase(name: str) -> bool:
    print(f"Checking for {name}")
    res = shutil.which(name)
    if res is not None:
        print(f"-- Checking for {name} - found")
    else:
        print(f"-- Checking for {name} - not found")
    return res is not None


Toolchain = "dict[str,bool]"
CFAMILY = "cfamily"
JDK = "jdk"


def testToolchainCase(name: str, _type: str, progs: "list[str]") -> "Optional[Toolchain]":
    print(f"Testing for {name}")
    res: Toolchain = {"name": name, "type": _type}
    _progs = []
    success = True
    for p in progs:
        r = testCase(p)
        if not r:
            success = False
        else:
            _progs.append({"cmd": p, "found": r})

    res["progs"] = _progs
    
    if success:
        print(f"Testing for {name} - succeeded")
        return res
    else:
        print(f"Testing for {name} - failed")
        return None


def determineToolchains(targetPlatform: str) -> "list[Toolchain]":

    res = []

    for conf in toolchainConfig:
        try:
            conf["platforms"].index(targetPlatform)
        except ValueError:
            continue
        _test_case = testToolchainCase(conf["name"], conf["type"], conf["progs"])
        if _test_case is not None:
            res.append(_test_case)

    return res


def determinePlatform() -> str:
    if sys.platform == "win32":
        return "windows"
    elif sys.platform == "darwin":
        return "macos"
    elif sys.platform == "linux":
        return "linux"


def main(__args):

    global toolchainConfig

    execPath: str = os.path.dirname(__file__)

    toolchainConfig = json.load(io.open(os.path.join(execPath, "default_toolchains.json"), "r"))

    parser = argparse.ArgumentParser(prog="autom-toolchain")
    parser.add_argument("--toolchain", choices=["msvc", "llvm", "gcc", "jdk"], nargs="?")
    parser.add_argument("--target-platform", choices=["windows", "macos", "linux", "ios", "android"], nargs="?")
    args = parser.parse_args(__args)

    if not args.target_platform:
        args.target_platform = determinePlatform()
    
    toolchain = determineToolchains(args.target_platform)

    json.dump(toolchain, io.open("./AUTOM.toolchain", "w"))


if __name__ == "__main__":
    sys.argv.pop(0)

    main(sys.argv)
