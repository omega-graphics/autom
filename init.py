import os,shutil
import sys,zipfile,tarfile
from urllib.request import urlretrieve

def run_python3(script:str):
    if sys.platform == "win32":
        os.system(f"py -3 {script}")
    else:
        os.system(f"python3 {script}")

def download(name:str,url:str,dest:str):
    print(f"Downloading {name} from :{url}")
    d = urlretrieve(url,dest)

def git(name:str,url:str,dest:str):
    print(f"Cloning {name} from :{url}")
    os.system(f"git clone {url} {dest}")

if shutil.which("cmake") is None:
    download("cmake","https://github.com/Kitware/CMake/releases/download/v3.20.5/cmake-3.20.5-macos-universal.tar.gz","./cmake.tar.gz")
    z = tarfile.open("./cmake.tar.gz")
    z.extractall("./cmake")
    z.close()

if shutil.which("ninja") is None:
    download("Ninja","https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-mac.zip","./ninja.zip")
    z = zipfile.ZipFile("./ninja.zip")
    z.extractall("./ninja-build")
    z.close()

if shutil.which("gn") is None:
    git("GN","https://gn.googlesource.com/gn","./gn-src")
    os.chdir("./gn-src")
    run_python3("./build/gen.py")
    os.system("\"./ninja-build/ninja\" -C ./out")
    
git("rapidjson","https://github.com/Tencent/rapidjson.git","./deps/rapidjson")    
git("libarchive","https://github.com/libarchive/libarchive.git","./deps/libarchive")    