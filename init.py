import os,shutil
import sys,zipfile,tarfile
from urllib.request import urlretrieve

is_win = sys.platform == "win32"
is_mac = sys.platform == "darwin"
is_linux = sys.platform == "linux"

def run_python3(script:str):
    if sys.platform == "win32":
        os.system(f"py -3 {script}")
    else:
        os.system(f"python3 {script}")

def download(name:str,url:str,dest:str):
    print(f"Downloading {name} from: {url}")
    d = urlretrieve(url,dest)

def git(name:str,url:str,dest:str):
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
    url:str
    if is_win:
        url = "https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-win.zip"
    else:
        url = "https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-mac.zip"
    download("Ninja",url,"./ninja.zip")
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
    
git("rapidjson","https://github.com/Tencent/rapidjson.git","./deps/rapidjson")    
#git("libarchive","https://github.com/libarchive/libarchive.git","./deps/libarchive")    

