import io
import os, shutil
import sys, zipfile, tarfile
from urllib.request import urlretrieve
import enum
from glob import glob
# from bin.autom_toolchain import determineToolchains

is_win = sys.platform == "win32"
is_mac = sys.platform == "darwin"
is_linux = sys.platform == "linux"


def run_python3(script:str):
    if sys.platform == "win32":
        os.system(f"py -3 {script}")
    else:
        os.system(f"python3 {script}")


def download(name: str, url: str, dest: str):
    print(f"Downloading {name} from: {url}")
    d = urlretrieve(url, dest)


def git(name: str, url: str, dest: str):
    print(f"Cloning {name} from: {url}")
    os.system(f"git clone {url} {dest}")

# if shutil.which("cmake") is None:
#     url:str
#     if is_win:
#         url = "https://github.com/Kitware/CMake/releases/download/v3.20.5/cmake-3.20.5-windows-x86_64.zip"
#     else:
#         url = "https://github.com/Kitware/CMake/releases/download/v3.20.5/cmake-3.20.5-macos-universal.tar.gz"

#     if is_win:
#         download("cmake",url,"./cmake.zip")
#         z = zipfile.ZipFile("./cmake.zip","r")
#         z.extractall("./cmake")
#         z.close()
#     else:
#         download("cmake",url,"./cmake.tar.gz")
#         z = tarfile.open("./cmake.tar.gz","r:*")
#         z.extractall("./cmake")
#         z.close()


if shutil.which("ninja") is None:
    url: str
    if is_win:
        url = "https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-win.zip"
    else:
        url = "https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-mac.zip"
    download("Ninja", url, "./ninja.zip")
    z = zipfile.ZipFile("./ninja.zip")
    z.extractall("./ninja-build")
    z.close()

# if shutil.which("gn") is None:
#     git("GN","https://gn.googlesource.com/gn","./gn-src")
#     run_python3("./build/gen.py")
#     if is_win:
#         os.system("bin\\vc-init && ninja-build\\ninja.exe -C ./gn-src/out")
#     else:
#         os.chdir("./gn-src")
#         os.system("\"./ninja-build/ninja\" -C ./out")
    
git("rapidjson", "https://github.com/Tencent/rapidjson.git","./deps/rapidjson")
git("pyyaml", "https://github.com/yaml/pyyaml.git", "./tm/pyyaml")



class NinjaBuildRuleTy(enum.Enum):
    COMPILE = 0,
    LINK_SHARED = 1,
    LINK_STATIC = 2,
    LINK_EXE = 3,


class NinjaBuildRule:
    ty: NinjaBuildRuleTy
    src: str
    output: str
    # deps:"list[str]"

    def __init__(self,ty:NinjaBuildRuleTy,src: str,output: str):
        self.ty = ty
        self.src = src
        self.output = output
        # self.deps = deps
        return


def make_ninja_build(file: str):

    def generate_build_rules(rules: "list[NinjaBuildRule]"):
        for r in rules:
            stream.write("\n")
            stream.write(f"build {r.output} : ")
            if r.ty == NinjaBuildRuleTy.COMPILE:
                stream.write(f"cxx {r.src}")
            elif r.ty == NinjaBuildRuleTy.LINK_SHARED:
                stream.write(f"so_ld {r.src}")
            elif r.ty == NinjaBuildRuleTy.LINK_EXE:
                stream.write(f"ld {r.src}")
            stream.write("\n")

    stream = io.open(file,"w")

    stream.write("ninja_minimum_required = 1.5\n\n")

    toolchains = determineToolchains()
    # Create CXX Rule
    stream.write("rule cxx\n command = ")
    if toolchains["LLVM"]["clang"]:
        stream.write("clang++ -std=c++17 -o $out -c $in\n")
    elif toolchains["GCC"]["g++"]:
        stream.write("g++ -std=c++17 -fPIC -o $out $in\n")

    # Create SO_LD Rule
    stream.write("rule so_ld\n command = ")
    if toolchains["LLVM"]["lld"]:
        stream.write("clang++ -shared -o $out $in\n")
    elif toolchains["GCC"]["ld"]:
        stream.write("g++ -std=c++17 -shared -o $out $in\n")

    # Create LD Rule
    stream.write("rule ld\n command = ")
    if toolchains["LLVM"]["lld"]:
        stream.write("clang++ -o $out $in\n")
    elif toolchains["GCC"]["ld"]:
        stream.write("g++ -std=c++17 -pie -o $out $in\n")
    
    stream.write("\n\n")

    _srcs = glob("./src/*.cpp") + glob("./src/parser/*.cpp") + glob("./src/gen/*.cpp") + glob("./src/bridge/*.cpp") + ["./src/driver/main.cpp"]

    rules = []

    outputs = []

    for ls in _srcs:
        s = os.path.basename(ls)
        src, ext = os.path.splitext(s)
        o = f"./out/obj/{os.path.basename(src)}.o"
        rules.append(NinjaBuildRule(NinjaBuildRuleTy.COMPILE, ls, o))
        outputs.append(o)
    rules.append(NinjaBuildRule(NinjaBuildRuleTy.LINK_EXE, " ".join(outputs), "./out/autom"))

    generate_build_rules(rules)


make_ninja_build("./build.ninja")

    
    
    
    

